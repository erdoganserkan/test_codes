/******************************************************************
 *****                                                        *****
 *****  Name: tcpip.c                                         *****
 *****  Ver.: 1.0                                             *****
 *****  Date: 07/05/2001                                      *****
 *****  Auth: Andreas Dannenberg                              *****
 *****        HTWK Leipzig                                    *****
 *****        university of applied sciences                  *****
 *****        Germany                                         *****
 *****  Func: implements the TCP/IP-stack and provides a      *****
 *****        simple API to the user                          *****
 *****                                                        *****
 ******************************************************************/

#include "tcpip.h"
#include "EMAC.h"         // Keil: Line added
#include <string.h>       // Keil: Line added
#include "LPC177x_8x.h"      // Keil: Register definition file for LPC2378
#include "lpc177x_8x_clkpwr.h"
#include "lpc177x_8x_pinsel.h"
#include "lpc177x_8x_timer.h"
#include "lpc177x_8x_nvic.h"
#include "lpc177x_8x_emac.h"


#if defined ( __CC_ARM   )
uint16_t __align(4) _TxFrame1[(ETH_HEADER_SIZE + IP_HEADER_SIZE + TCP_HEADER_SIZE + MAX_TCP_TX_DATA_SIZE)/2];
uint16_t __align(4) _TxFrame2[(ETH_HEADER_SIZE + MAX_ETH_TX_DATA_SIZE)/2];
uint16_t __align(4) _RxTCPBuffer[MAX_TCP_RX_DATA_SIZE/2]; // space for incoming TCP-data
#elif defined ( __ICCARM__ )
#pragma data_alignment=4
uint16_t _TxFrame1[(ETH_HEADER_SIZE + IP_HEADER_SIZE + TCP_HEADER_SIZE + MAX_TCP_TX_DATA_SIZE)/2];
#pragma data_alignment=4
uint16_t _TxFrame2[(ETH_HEADER_SIZE + MAX_ETH_TX_DATA_SIZE)/2];
#pragma data_alignment=4
uint16_t _RxTCPBuffer[MAX_TCP_RX_DATA_SIZE/2]; // space for incoming TCP-data
#else
uint16_t __attribute__ ((aligned (4))) _TxFrame1[(ETH_HEADER_SIZE + IP_HEADER_SIZE + TCP_HEADER_SIZE + MAX_TCP_TX_DATA_SIZE)/2];
uint16_t __attribute__ ((aligned (4))) _TxFrame2[(ETH_HEADER_SIZE + MAX_ETH_TX_DATA_SIZE)/2];
uint16_t __attribute__ ((aligned (4))) _RxTCPBuffer[MAX_TCP_RX_DATA_SIZE/2]; // space for incoming TCP-data
#endif

uint16_t TxFrame1Size;              // bytes to send in TxFrame1
uint8_t TxFrame2Size;               // bytes to send in TxFrame2
uint8_t TransmitControl;
uint8_t TCPFlags;
uint16_t TCPRemotePort;

uint16_t RemoteMAC[3];              // MAC and IP of current TCP-session
uint16_t RemoteIP[2];


// constants
const uint16_t MyIP[] =                    // "MYIP1.MYIP2.MYIP3.MYIP4"
{
  MYIP_1 + (MYIP_2 << 8),                        // use 'unsigned int' to
  MYIP_3 + (MYIP_4 << 8)                         // achieve word alignment
};

const uint16_t SubnetMask[] =              // "SUBMASK1.SUBMASK2.SUBMASK3.SUBMASK4"
{
  SUBMASK_1 + (SUBMASK_2 << 8),                  // use 'unsigned int' to
  SUBMASK_3 + (SUBMASK_4 << 8)                   // achieve word alignment
};

const uint16_t GatewayIP[] =               // "GWIP1.GWIP2.GWIP3.GWIP4"
{
  GWIP_1 + (GWIP_2 << 8),                        // use 'unsigned int' to
  GWIP_3 + (GWIP_4 << 8)                         // achieve word alignment
};
const uint8_t MyMAC[6] =   // "M1-M2-M3-M4-M5-M6"
{
  MYMAC_1, MYMAC_2, MYMAC_3,
  MYMAC_4, MYMAC_5, MYMAC_6
};


// easyWEB's internal variables
TTCPStateMachine TCPStateMachine;         // perhaps the most important var at all ;-)
TLastFrameSent LastFrameSent;             // retransmission type

uint16_t ISNGenHigh;                // upper word of our Initial Sequence Number
unsigned long TCPSeqNr;                   // next sequence number to send
unsigned long TCPUNASeqNr;                // last unaknowledged sequence number
                                                 // incremented AFTER sending data
unsigned long TCPAckNr;                   // next seq to receive and ack to send
                                                 // incremented AFTER receiving data
uint8_t TCPTimer;                   // inc'd each 262ms
uint8_t RetryCounter;               // nr. of retransmissions

// properties of the just received frame
uint16_t RecdFrameLength;           // EMAC reported frame length
uint16_t RecdFrameMAC[3];           // 48 bit MAC
uint16_t RecdFrameIP[2];            // 32 bit IP
uint16_t RecdIPFrameLength;         // 16 bit IP packet length


#define TxFrame1      ((uint8_t *)_TxFrame1)
#define TxFrame2      ((uint8_t *)_TxFrame2)
#define RxTCPBuffer   ((uint8_t *)_RxTCPBuffer)


// easyWEB-API function
// initalizes the LAN-controller, reset flags, starts timer-ISR
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void TCPLowLevelInit(void)
{
	Init_EMAC();

	TransmitControl = 0;
	TCPFlags = 0;
	TCPStateMachine = CLOSED;
	SocketStatus = 0;
}

// easyWEB-API function
// does a passive open (listen on 'MyIP:TCPLocalPort' for an incoming
// connection)
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void TCPPassiveOpen(void)
{
  if (TCPStateMachine == CLOSED)
  {
    TCPFlags &= ~TCP_ACTIVE_OPEN;                // let's do a passive open!
    TCPStateMachine = LISTENING;
    SocketStatus = SOCK_ACTIVE;                  // reset, socket now active
  }
}

// easyWEB-API function
// does an active open (tries to establish a connection between
// 'MyIP:TCPLocalPort' and 'RemoteIP:TCPRemotePort')
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void TCPActiveOpen(void)
{
  if ((TCPStateMachine == CLOSED) || (TCPStateMachine == LISTENING))
  {
    TCPFlags |= TCP_ACTIVE_OPEN;                 // let's do an active open!
    TCPFlags &= ~IP_ADDR_RESOLVED;               // we haven't opponents MAC yet

    PrepareARP_REQUEST();                        // ask for MAC by sending a broadcast
    LastFrameSent = ARP_REQUEST;
    TCPStartRetryTimer();
    SocketStatus = SOCK_ACTIVE;                  // reset, socket now active
  }
}

// easyWEB-API function
// closes an open connection
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void TCPClose(void)
{
  switch (TCPStateMachine)
  {
    case LISTENING :
    case SYN_SENT :
    {
      TCPStateMachine = CLOSED;
      TCPFlags = 0;
      SocketStatus = 0;
      break;
    }
    case SYN_RECD :
    case ESTABLISHED :
    {
      TCPFlags |= TCP_CLOSE_REQUESTED;
      break;
    }
    default:
    	break;
  }
}

// easyWEB-API function
// releases the receive-buffer and allows easyWEB to store new data
// NOTE: rx-buffer MUST be released periodically, else the other TCP
//       get no ACKs for the data it sent
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void TCPReleaseRxBuffer(void)
{
  SocketStatus &= ~SOCK_DATA_AVAILABLE;
}

// easyWEB-API function
// transmitts data stored in 'TCP_TX_BUF'
// NOTE: * number of bytes to transmit must have been written to 'TCPTxDataCount'
//       * data-count MUST NOT exceed 'MAX_TCP_TX_DATA_SIZE'
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void TCPTransmitTxBuffer(void)
{
  if ((TCPStateMachine == ESTABLISHED) || (TCPStateMachine == CLOSE_WAIT))
    if (SocketStatus & SOCK_TX_BUF_RELEASED)
    {
      SocketStatus &= ~SOCK_TX_BUF_RELEASED;               // occupy tx-buffer
      TCPUNASeqNr += TCPTxDataCount;                       // advance UNA

      TxFrame1Size = ETH_HEADER_SIZE + IP_HEADER_SIZE + TCP_HEADER_SIZE + TCPTxDataCount;
      TransmitControl |= SEND_FRAME1;

      LastFrameSent = TCP_DATA_FRAME;
      TCPStartRetryTimer();
    }
}

// Reads the length of the received ethernet frame and checks if the
// destination address is a broadcast message or not
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
unsigned int IsBroadcast(void) {
  uint16_t RecdDestMAC[3];         // 48 bit MAC
  RecdFrameLength = StartReadFrame();
  ReadFrame_EMAC(&RecdDestMAC,6);               // receive DA to see if it was a broadcast
  ReadFrame_EMAC(&RecdFrameMAC,6);               // store SA (for our answer)

  if ((RecdDestMAC[0] == 0xFFFF) &&
      (RecdDestMAC[1] == 0xFFFF) &&
      (RecdDestMAC[2] == 0xFFFF)) {
    return(1);
  } else {
    return (0);
  }
}


// easyWEB's 'main()'-function
// must be called from user program periodically (the often - the better)
// handles network, TCP/IP-stack and user events
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void DoNetworkStuff(void)
{
	if (CheckFrameReceived())                      // Packet received
	{
		if (IsBroadcast())
		{
			ProcessEthBroadcastFrame();
		} 
		else 
		{
			ProcessEthIAFrame();
		}
		
		EndReadFrame();                              // release buffer in ethernet controller
	}

	if (TCPFlags & TCP_TIMER_RUNNING)
	{
		if (TCPFlags & TIMER_TYPE_RETRY)
		{
			if (TCPTimer > RETRY_TIMEOUT)
			{
				TCPRestartTimer();                       // set a new timeout

				if (RetryCounter)
				{
					TCPHandleRetransmission();             // resend last frame
					RetryCounter--;
				}
				else
				{
					TCPStopTimer();
					TCPHandleTimeout();
				}
			}
		}
		else if (TCPTimer > FIN_TIMEOUT)
		{
			TCPStateMachine = CLOSED;
			TCPFlags = 0;                              // reset all flags, stop retransmission...
			SocketStatus &= SOCK_DATA_AVAILABLE;       // clear all flags but data available
		}
	}
	
	switch (TCPStateMachine)
	{
		case CLOSED :
		case LISTENING :
		{
			if (TCPFlags & TCP_ACTIVE_OPEN)            // stack has to open a connection?
			{
				if (TCPFlags & IP_ADDR_RESOLVED)         // IP resolved?
				{
					if (!(TransmitControl & SEND_FRAME2))  // buffer free?
					{
						TCPSeqNr = ((unsigned long)ISNGenHigh << 16) | (LPC_TIM0->TC & 0xFFFF);  // Keil: changed from TAR to T0TC;
						// set local ISN
						TCPUNASeqNr = TCPSeqNr;
						TCPAckNr = 0;                                       // we don't know what to ACK!
						TCPUNASeqNr++;                                      // count SYN as a byte

						PrepareTCP_FRAME(TCP_CODE_SYN);                     // send SYN frame

						LastFrameSent = TCP_SYN_FRAME;

						TCPStartRetryTimer();                               // we NEED a retry-timeout

						TCPStateMachine = SYN_SENT;
					}
				}
			}
			
			break;
		}
		
		case SYN_RECD :
		case ESTABLISHED :
		{
			if (TCPFlags & TCP_CLOSE_REQUESTED)                  // user has user initated a close?
			{
				if (!(TransmitControl & (SEND_FRAME2 | SEND_FRAME1)))   // buffers free?
				{   
					if (TCPSeqNr == TCPUNASeqNr)                          // all data ACKed?
					{
						TCPUNASeqNr++;

						PrepareTCP_FRAME(TCP_CODE_FIN | TCP_CODE_ACK);

						LastFrameSent = TCP_FIN_FRAME;

						TCPStartRetryTimer();

						TCPStateMachine = FIN_WAIT_1;
					}
				}
			}
			
			break;
		}
		case CLOSE_WAIT :
		{
			if (!(TransmitControl & (SEND_FRAME2 | SEND_FRAME1)))     // buffers free?
			{
				if (TCPSeqNr == TCPUNASeqNr)                            // all data ACKed?
				{
					TCPUNASeqNr++;                                        // count FIN as a byte

					PrepareTCP_FRAME(TCP_CODE_FIN | TCP_CODE_ACK);        // we NEED a retry-timeout

					LastFrameSent = TCP_FIN_FRAME;                        // time to say goodbye...

					TCPStartRetryTimer();

					TCPStateMachine = LAST_ACK;
				}
			}
			
			break;
		}
		
		default:
			break;
	}

	if (TransmitControl & SEND_FRAME2)
	{
		if (Rdy4Tx())                                // NOTE: when using a very fast MCU, maybe
			SendFrame2();                              // the EMAC isn't ready yet, include
		else 
		{                                       // a kind of timer or counter here
			TCPStateMachine = CLOSED;
			SocketStatus = SOCK_ERR_ETHERNET;          // indicate an error to user
			TCPFlags = 0;                              // clear all flags, stop timers etc.
		}

		TransmitControl &= ~SEND_FRAME2;             // clear tx-flag
	}

	if (TransmitControl & SEND_FRAME1)
	{
		PrepareTCP_DATA_FRAME();                     // build frame w/ actual SEQ, ACK....

		if (Rdy4Tx())                                // EMAC ready to accept our frame?
			SendFrame1();                              // (see note above)
		else 
		{
			TCPStateMachine = CLOSED;
			SocketStatus = SOCK_ERR_ETHERNET;          // indicate an error to user
			TCPFlags = 0;                              // clear all flags, stop timers etc.
		}

		TransmitControl &= ~SEND_FRAME1;             // clear tx-flag
	}
}

// easyWEB internal function
// handles an incoming broadcast frame
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void ProcessEthBroadcastFrame(void)
{
  uint16_t TargetIP[2];

  if (ReadHalfWordBE_EMAC() == FRAME_ARP)           // get frame type, check for ARP
    if (ReadHalfWordBE_EMAC() == HARDW_ETH10)       // Ethernet frame
      if (ReadHalfWordBE_EMAC() == FRAME_IP)        // check protocol
        if (ReadHalfWordBE_EMAC() == IP_HLEN_PLEN)  // check HLEN, PLEN
          if (ReadHalfWordBE_EMAC() == OP_ARP_REQUEST)
          {
            DummyReadFrame_EMAC(6);              // ignore sender's hardware address
            ReadFrame_EMAC(&RecdFrameIP, 4); // read sender's protocol address
            DummyReadFrame_EMAC(6);              // ignore target's hardware address
            ReadFrame_EMAC(&TargetIP, 4);    // read target's protocol address
            if (!memcmp(&MyIP, &TargetIP, 4))    // is it for us?
              PrepareARP_ANSWER();               // yes->create ARP_ANSWER frame
          }
}

// easyWEB internal function
// handles an incoming frame that passed EMAC's address filter
// (individual addressed = IA)
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void ProcessEthIAFrame(void)
{
  uint16_t TargetIP[2];
  uint8_t ProtocolType;

  switch (ReadHalfWordBE_EMAC())                     // get frame type
  {
    case FRAME_ARP :                             // check for ARP
    {
      if ((TCPFlags & (TCP_ACTIVE_OPEN | IP_ADDR_RESOLVED)) == TCP_ACTIVE_OPEN)
        if (ReadHalfWordBE_EMAC() == HARDW_ETH10)         // check for the right prot. etc.
          if (ReadHalfWordBE_EMAC() == FRAME_IP)
            if (ReadHalfWordBE_EMAC() == IP_HLEN_PLEN)
              if (ReadHalfWordBE_EMAC() == OP_ARP_ANSWER)
              {
                TCPStopTimer();                       // OK, now we've the MAC we wanted ;-)
                ReadFrame_EMAC(&RemoteMAC, 6);    // extract opponents MAC
                TCPFlags |= IP_ADDR_RESOLVED;
              }
      break;
    }
    case FRAME_IP :                                        // check for IP-type
    {
      if ((ReadHalfWordBE_EMAC() & 0xFF00 ) == IP_VER_IHL)     // IPv4, IHL=5 (20 Bytes Header)
      {                                                    // ignore Type Of Service
        RecdIPFrameLength = ReadHalfWordBE_EMAC();             // get IP frame's length
        ReadHalfWordBE_EMAC();                                 // ignore identification

        if (!(ReadHalfWordBE_EMAC() & (IP_FLAG_MOREFRAG | IP_FRAGOFS_MASK)))  // only unfragm. frames
        {
          ProtocolType = ReadHalfWordBE_EMAC() & 0xFF;         // get protocol, ignore TTL
          ReadHalfWordBE_EMAC();                               // ignore checksum
          ReadFrame_EMAC(&RecdFrameIP, 4);              // get source IP
          ReadFrame_EMAC(&TargetIP, 4);                 // get destination IP

          if (!memcmp(&MyIP, &TargetIP, 4))                // is it for us?
            switch (ProtocolType) {
              case PROT_ICMP : { ProcessICMPFrame(); break; }
              case PROT_TCP  : { ProcessTCPFrame(); break; }
              case PROT_UDP  : break;                      // not implemented!
            }
        }
      }
      break;
    }
  }
}

// easyWEB internal function
// we've just rec'd an ICMP-frame (Internet Control Message Protocol)
// check what to do and branch to the appropriate sub-function
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void ProcessICMPFrame(void)
{
  uint16_t ICMPTypeAndCode;

  ICMPTypeAndCode = ReadHalfWordBE_EMAC();           // get Message Type and Code
  ReadHalfWordBE_EMAC();                             // ignore ICMP checksum

  switch (ICMPTypeAndCode >> 8) {                // check type
    case ICMP_ECHO :                             // is echo request?
    {
      PrepareICMP_ECHO_REPLY();                  // echo as much as we can...
      break;
    }
  }
}

// easyWEB internal function
// we've just rec'd an TCP-frame (Transmission Control Protocol)
// this function mainly implements the TCP state machine according to RFC793
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void ProcessTCPFrame(void)
{
  uint16_t TCPSegSourcePort;               // segment's source port
  uint16_t TCPSegDestPort;                 // segment's destination port
  uint32_t TCPSegSeq;                       // segment's sequence number
  uint32_t TCPSegAck;                       // segment's acknowledge number
  uint16_t TCPCode;                        // TCP code and header length
  uint8_t TCPHeaderSize;                   // real TCP header length
  uint16_t NrOfDataBytes;                  // real number of data

  TCPSegSourcePort = ReadHalfWordBE_EMAC();                    // get ports
  TCPSegDestPort = ReadHalfWordBE_EMAC();

  if (TCPSegDestPort != TCPLocalPort) return;              // drop segment if port doesn't match

  TCPSegSeq = (uint32_t)ReadHalfWordBE_EMAC() << 16;      // get segment sequence nr.
  TCPSegSeq |= ReadHalfWordBE_EMAC();

  TCPSegAck = (uint32_t)ReadHalfWordBE_EMAC() << 16;      // get segment acknowledge nr.
  TCPSegAck |= ReadHalfWordBE_EMAC();

  TCPCode = ReadHalfWordBE_EMAC();                             // get control bits, header length...

  TCPHeaderSize = (TCPCode & DATA_OFS_MASK) >> 10;         // header length in bytes
  NrOfDataBytes = RecdIPFrameLength - IP_HEADER_SIZE - TCPHeaderSize;     // seg. text length

  if (NrOfDataBytes > MAX_TCP_RX_DATA_SIZE) return;        // packet too large for us :...-(

  if (TCPHeaderSize > TCP_HEADER_SIZE)                     // ignore options if any
    DummyReadFrame_EMAC(TCPHeaderSize - TCP_HEADER_SIZE);

  switch (TCPStateMachine)                                 // implement the TCP state machine
  {
    case CLOSED :
    {
      if (!(TCPCode & TCP_CODE_RST))
      {
        TCPRemotePort = TCPSegSourcePort;
        memcpy(&RemoteMAC, &RecdFrameMAC, 6);              // save opponents MAC and IP
        memcpy(&RemoteIP, &RecdFrameIP, 4);                // for later use

        if (TCPCode & TCP_CODE_ACK)                        // make the reset sequence
        {                                                  // acceptable to the other
          TCPSeqNr = TCPSegAck;                            // TCP
          PrepareTCP_FRAME(TCP_CODE_RST);
        }
        else
        {
          TCPSeqNr = 0;
          TCPAckNr = TCPSegSeq + NrOfDataBytes;
          if (TCPCode & (TCP_CODE_SYN | TCP_CODE_FIN)) TCPAckNr++;
          PrepareTCP_FRAME(TCP_CODE_RST | TCP_CODE_ACK);
        }
      }
      break;
    }
    case LISTENING :
    {
      if (!(TCPCode & TCP_CODE_RST))                       // ignore segment containing RST
      {
        TCPRemotePort = TCPSegSourcePort;
        memcpy(&RemoteMAC, &RecdFrameMAC, 6);              // save opponents MAC and IP
        memcpy(&RemoteIP, &RecdFrameIP, 4);                // for later use

        if (TCPCode & TCP_CODE_ACK)                        // reset a bad
        {                                                  // acknowledgement
          TCPSeqNr = TCPSegAck;
          PrepareTCP_FRAME(TCP_CODE_RST);
        }
        else if (TCPCode & TCP_CODE_SYN)
        {
          TCPAckNr = TCPSegSeq + 1;                           // get remote ISN, next byte we expect
            TCPSeqNr = ((unsigned long)ISNGenHigh << 16) | (LPC_TIM0->TC & 0xFFFF);  // Keil: changed from TAR to T0TC;
                                                              // set local ISN
          TCPUNASeqNr = TCPSeqNr + 1;                         // one byte out -> increase by one
          PrepareTCP_FRAME(TCP_CODE_SYN | TCP_CODE_ACK);
          LastFrameSent = TCP_SYN_ACK_FRAME;
          TCPStartRetryTimer();
          TCPStateMachine = SYN_RECD;
        }
      }
      break;
    }
    case SYN_SENT :
    {
      if (memcmp(&RemoteIP, &RecdFrameIP, 4)) break;  // drop segment if its IP doesn't belong
                                                      // to current session

      if (TCPSegSourcePort != TCPRemotePort) break;   // drop segment if port doesn't match

      if (TCPCode & TCP_CODE_ACK)                // ACK field significant?
        if (TCPSegAck != TCPUNASeqNr)            // is our ISN ACKed?
        {
          if (!(TCPCode & TCP_CODE_RST))
          {
            TCPSeqNr = TCPSegAck;
            PrepareTCP_FRAME(TCP_CODE_RST);
          }
          break;                                 // drop segment
        }

      if (TCPCode & TCP_CODE_RST)                // RST??
      {
        if (TCPCode & TCP_CODE_ACK)              // if ACK was acceptable, reset
        {                                        // connection
          TCPStateMachine = CLOSED;
          TCPFlags = 0;                          // reset all flags, stop retransmission...
          SocketStatus = SOCK_ERR_CONN_RESET;
        }
        break;                                   // drop segment
      }

      if (TCPCode & TCP_CODE_SYN)                // SYN??
      {
        TCPAckNr = TCPSegSeq;                    // get opponents ISN
        TCPAckNr++;                              // inc. by one...

        if (TCPCode & TCP_CODE_ACK)
        {
          TCPStopTimer();                        // stop retransmission, other TCP got our SYN
          TCPSeqNr = TCPUNASeqNr;                // advance our sequence number

          PrepareTCP_FRAME(TCP_CODE_ACK);        // ACK this ISN
          TCPStateMachine = ESTABLISHED;
          SocketStatus |= SOCK_CONNECTED;
          SocketStatus |= SOCK_TX_BUF_RELEASED;  // user may send data now :-)
        }
        else
        {
          TCPStopTimer();
          PrepareTCP_FRAME(TCP_CODE_SYN | TCP_CODE_ACK);   // our SYN isn't ACKed yet,
          LastFrameSent = TCP_SYN_ACK_FRAME;               // now continue with sending
          TCPStartRetryTimer();                            // SYN_ACK frames
          TCPStateMachine = SYN_RECD;
        }
      }
      break;
    }
    default :
    {
      if (memcmp(&RemoteIP, &RecdFrameIP, 4)) break;  // drop segment if IP doesn't belong
                                                      // to current session

      if (TCPSegSourcePort != TCPRemotePort) break;   // drop segment if port doesn't match

      if (TCPSegSeq != TCPAckNr) break;               // drop if it's not the segment we expect

      if (TCPCode & TCP_CODE_RST)                // RST??
      {
        TCPStateMachine = CLOSED;                // close the state machine
        TCPFlags = 0;                            // reset all flags, stop retransmission...
        SocketStatus = SOCK_ERR_CONN_RESET;      // indicate an error to user
        break;
      }

      if (TCPCode & TCP_CODE_SYN)                // SYN??
      {
        PrepareTCP_FRAME(TCP_CODE_RST);          // is NOT allowed here! send a reset,
        TCPStateMachine = CLOSED;                // close connection...
        TCPFlags = 0;                            // reset all flags, stop retransmission...
        SocketStatus = SOCK_ERR_REMOTE;          // fatal error!
        break;                                   // ...and drop the frame
      }

      if (!(TCPCode & TCP_CODE_ACK)) break;      // drop segment if the ACK bit is off

      if (TCPSegAck == TCPUNASeqNr)              // is our last data sent ACKed?
      {
        TCPStopTimer();                          // stop retransmission
        TCPSeqNr = TCPUNASeqNr;                  // advance our sequence number

        switch (TCPStateMachine)                 // change state if necessary
        {
          case SYN_RECD :                        // ACK of our SYN?
          {
            TCPStateMachine = ESTABLISHED;       // user may send data now :-)
            SocketStatus |= SOCK_CONNECTED;
            break;
          }
          case FIN_WAIT_1 :
          {
        	  TCPStateMachine = FIN_WAIT_2;
        	  break;
          } // ACK of our FIN?
          case CLOSING :
          {
        	  TCPStateMachine = TIME_WAIT;
        	  break;
          }  // ACK of our FIN?
          case LAST_ACK :                                            // ACK of our FIN?
          {
            TCPStateMachine = CLOSED;
            TCPFlags = 0;                        // reset all flags, stop retransmission...
            SocketStatus &= SOCK_DATA_AVAILABLE; // clear all flags but data available
            break;
          }
          case TIME_WAIT :
          {
            PrepareTCP_FRAME(TCP_CODE_ACK);      // ACK a retransmission of remote FIN
            TCPRestartTimer();                   // restart TIME_WAIT timeout
            break;
          }
          default:
        	  break;
        }

        if (TCPStateMachine == ESTABLISHED)      // if true, give the frame buffer back
          SocketStatus |= SOCK_TX_BUF_RELEASED;  // to user
      }

      if ((TCPStateMachine == ESTABLISHED) || (TCPStateMachine == FIN_WAIT_1) || (TCPStateMachine == FIN_WAIT_2))
        if (NrOfDataBytes)                                 // data available?
          if (!(SocketStatus & SOCK_DATA_AVAILABLE))       // rx data-buffer empty?
          {
            DummyReadFrame_EMAC(6);                        // ignore window, checksum, urgent pointer
            ReadFrame_EMAC(RxTCPBuffer, NrOfDataBytes);// fetch data and
            TCPRxDataCount = NrOfDataBytes;                // ...tell the user...
            SocketStatus |= SOCK_DATA_AVAILABLE;           // indicate the new data to user
            TCPAckNr += NrOfDataBytes;
            PrepareTCP_FRAME(TCP_CODE_ACK);                // ACK rec'd data
          }

      if (TCPCode & TCP_CODE_FIN)                // FIN??
      {
        switch (TCPStateMachine)
        {
          case SYN_RECD :
          case ESTABLISHED :
          {
            TCPStateMachine = CLOSE_WAIT;
            break;
          }
          case FIN_WAIT_1 :
          {                                      // if our FIN was ACKed, we automatically
            TCPStateMachine = CLOSING;           // enter FIN_WAIT_2 (look above) and therefore
            SocketStatus &= ~SOCK_CONNECTED;     // TIME_WAIT
            break;
          }
          case FIN_WAIT_2 :
          {
            TCPStartTimeWaitTimer();
            TCPStateMachine = TIME_WAIT;
            SocketStatus &= ~SOCK_CONNECTED;
            break;
          }
          case TIME_WAIT :
          {
            TCPRestartTimer();
            break;
          }
          default:
        	  break;
        }
        TCPAckNr++;                              // ACK remote's FIN flag
        PrepareTCP_FRAME(TCP_CODE_ACK);
      }
      break;
    }
  }
}

// easyWEB internal function
// prepares the TxFrame2-buffer to send an ARP-request
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void PrepareARP_REQUEST(void)
{
  // Ethernet
  memset(&TxFrame2[ETH_DA_OFS], (char)0xFF, 6);                  // we don't know opposites MAC!
  memcpy(&TxFrame2[ETH_SA_OFS], &MyMAC, 6);
  *(uint16_t *)&TxFrame2[ETH_TYPE_OFS] = SWAPB(FRAME_ARP);

  // ARP
  *(uint16_t *)&TxFrame2[ARP_HARDW_OFS] = SWAPB(HARDW_ETH10);
  *(uint16_t *)&TxFrame2[ARP_PROT_OFS] = SWAPB(FRAME_IP);
  *(uint16_t *)&TxFrame2[ARP_HLEN_PLEN_OFS] = SWAPB(IP_HLEN_PLEN);
  *(uint16_t *)&TxFrame2[ARP_OPCODE_OFS] = SWAPB(OP_ARP_REQUEST);
  memcpy(&TxFrame2[ARP_SENDER_HA_OFS], &MyMAC, 6);
  memcpy(&TxFrame2[ARP_SENDER_IP_OFS], &MyIP, 4);
  memset(&TxFrame2[ARP_TARGET_HA_OFS], 0x00, 6);           // we don't know opposites MAC!

  if (((RemoteIP[0] ^ MyIP[0]) & SubnetMask[0]) || ((RemoteIP[1] ^ MyIP[1]) & SubnetMask[1]))
    memcpy(&TxFrame2[ARP_TARGET_IP_OFS], &GatewayIP, 4);   // IP not in subnet, use gateway
  else
    memcpy(&TxFrame2[ARP_TARGET_IP_OFS], &RemoteIP, 4);    // other IP is next to us...

  TxFrame2Size = ETH_HEADER_SIZE + ARP_FRAME_SIZE;
  TransmitControl |= SEND_FRAME2;
}

// easyWEB internal function
// prepares the TxFrame2-buffer to send an ARP-answer (reply)
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void PrepareARP_ANSWER(void)
{
  // Ethernet
  memcpy(&TxFrame2[ETH_DA_OFS], &RecdFrameMAC, 6);
  memcpy(&TxFrame2[ETH_SA_OFS], &MyMAC, 6);
  *(uint16_t *)&TxFrame2[ETH_TYPE_OFS] = SWAPB(FRAME_ARP);

  // ARP
  *(uint16_t *)&TxFrame2[ARP_HARDW_OFS] = SWAPB(HARDW_ETH10);
  *(uint16_t *)&TxFrame2[ARP_PROT_OFS] = SWAPB(FRAME_IP);
  *(uint16_t *)&TxFrame2[ARP_HLEN_PLEN_OFS] = SWAPB(IP_HLEN_PLEN);
  *(uint16_t *)&TxFrame2[ARP_OPCODE_OFS] = SWAPB(OP_ARP_ANSWER);
  memcpy(&TxFrame2[ARP_SENDER_HA_OFS], &MyMAC, 6);
  memcpy(&TxFrame2[ARP_SENDER_IP_OFS], &MyIP, 4);
  memcpy(&TxFrame2[ARP_TARGET_HA_OFS], &RecdFrameMAC, 6);
  memcpy(&TxFrame2[ARP_TARGET_IP_OFS], &RecdFrameIP, 4);

  TxFrame2Size = ETH_HEADER_SIZE + ARP_FRAME_SIZE;
  TransmitControl |= SEND_FRAME2;
}

// easyWEB internal function
// prepares the TxFrame2-buffer to send an ICMP-echo-reply
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void PrepareICMP_ECHO_REPLY(void)
{
  uint16_t ICMPDataCount;

  if (RecdIPFrameLength > MAX_ETH_TX_DATA_SIZE)                      // don't overload TX-buffer
    ICMPDataCount = MAX_ETH_TX_DATA_SIZE - IP_HEADER_SIZE - ICMP_HEADER_SIZE;
  else
    ICMPDataCount = RecdIPFrameLength - IP_HEADER_SIZE - ICMP_HEADER_SIZE;

  // Ethernet
  memcpy(&TxFrame2[ETH_DA_OFS], &RecdFrameMAC, 6);
  memcpy(&TxFrame2[ETH_SA_OFS], &MyMAC, 6);
  *(uint16_t *)&TxFrame2[ETH_TYPE_OFS] = SWAPB(FRAME_IP);

  // IP
  *(uint16_t *)&TxFrame2[IP_VER_IHL_TOS_OFS] = SWAPB(IP_VER_IHL);
  WriteWBE(&TxFrame2[IP_TOTAL_LENGTH_OFS], IP_HEADER_SIZE + ICMP_HEADER_SIZE + ICMPDataCount);
  *(uint16_t *)&TxFrame2[IP_IDENT_OFS] = 0;
  *(uint16_t *)&TxFrame2[IP_FLAGS_FRAG_OFS] = 0;
  *(uint16_t *)&TxFrame2[IP_TTL_PROT_OFS] = SWAPB((DEFAULT_TTL << 8) | PROT_ICMP);
  *(uint16_t *)&TxFrame2[IP_HEAD_CHKSUM_OFS] = 0;
  memcpy(&TxFrame2[IP_SOURCE_OFS], &MyIP, 4);
  memcpy(&TxFrame2[IP_DESTINATION_OFS], &RecdFrameIP, 4);
  *(uint16_t *)&TxFrame2[IP_HEAD_CHKSUM_OFS] = CalcChecksum(&TxFrame2[IP_VER_IHL_TOS_OFS], IP_HEADER_SIZE, 0);

  // ICMP
  *(uint16_t *)&TxFrame2[ICMP_TYPE_CODE_OFS] = SWAPB(ICMP_ECHO_REPLY << 8);
  *(uint16_t *)&TxFrame2[ICMP_CHKSUM_OFS] = 0;                   // initialize checksum field

  ReadFrame_EMAC(&TxFrame2[ICMP_DATA_OFS], ICMPDataCount);        // get data to echo...
  *(uint16_t *)&TxFrame2[ICMP_CHKSUM_OFS] = CalcChecksum(&TxFrame2[IP_DATA_OFS], ICMPDataCount + ICMP_HEADER_SIZE, 0);

  TxFrame2Size = ETH_HEADER_SIZE + IP_HEADER_SIZE + ICMP_HEADER_SIZE + ICMPDataCount;
  TransmitControl |= SEND_FRAME2;
}

// easyWEB internal function
// prepares the TxFrame2-buffer to send a general TCP frame
// the TCPCode-field is passed as an argument
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void PrepareTCP_FRAME(uint16_t TCPCode)
{
  // Ethernet
  memcpy(&TxFrame2[ETH_DA_OFS], &RemoteMAC, 6);
  memcpy(&TxFrame2[ETH_SA_OFS], &MyMAC, 6);
  *(uint16_t *)&TxFrame2[ETH_TYPE_OFS] = SWAPB(FRAME_IP);

  // IP
  *(uint16_t *)&TxFrame2[IP_VER_IHL_TOS_OFS] = SWAPB(IP_VER_IHL | IP_TOS_D);

  if (TCPCode & TCP_CODE_SYN)                    // if SYN, we want to use the MSS option
    *(uint16_t *)&TxFrame2[IP_TOTAL_LENGTH_OFS] = SWAPB(IP_HEADER_SIZE + TCP_HEADER_SIZE + TCP_OPT_MSS_SIZE);
  else
    *(uint16_t *)&TxFrame2[IP_TOTAL_LENGTH_OFS] = SWAPB(IP_HEADER_SIZE + TCP_HEADER_SIZE);

  *(uint16_t *)&TxFrame2[IP_IDENT_OFS] = 0;
  *(uint16_t *)&TxFrame2[IP_FLAGS_FRAG_OFS] = 0;
  *(uint16_t *)&TxFrame2[IP_TTL_PROT_OFS] = SWAPB((DEFAULT_TTL << 8) | PROT_TCP);
  *(uint16_t *)&TxFrame2[IP_HEAD_CHKSUM_OFS] = 0;
  memcpy(&TxFrame2[IP_SOURCE_OFS], &MyIP, 4);
  memcpy(&TxFrame2[IP_DESTINATION_OFS], &RemoteIP, 4);
  *(uint16_t *)&TxFrame2[IP_HEAD_CHKSUM_OFS] = CalcChecksum(&TxFrame2[IP_VER_IHL_TOS_OFS], IP_HEADER_SIZE, 0);

  // TCP
  WriteWBE(&TxFrame2[TCP_SRCPORT_OFS], TCPLocalPort);
  WriteWBE(&TxFrame2[TCP_DESTPORT_OFS], TCPRemotePort);

  WriteDWBE(&TxFrame2[TCP_SEQNR_OFS], TCPSeqNr);
  WriteDWBE(&TxFrame2[TCP_ACKNR_OFS], TCPAckNr);

  *(uint16_t *)&TxFrame2[TCP_WINDOW_OFS] = SWAPB(MAX_TCP_RX_DATA_SIZE);    // data bytes to accept
  *(uint16_t *)&TxFrame2[TCP_CHKSUM_OFS] = 0;             // initalize checksum
  *(uint16_t *)&TxFrame2[TCP_URGENT_OFS] = 0;

  if (TCPCode & TCP_CODE_SYN)                    // if SYN, we want to use the MSS option
  {
    *(uint16_t *)&TxFrame2[TCP_DATA_CODE_OFS] = SWAPB(0x6000 | TCPCode);   // TCP header length = 24
    *(uint16_t *)&TxFrame2[TCP_DATA_OFS] = SWAPB(TCP_OPT_MSS);             // MSS option
    *(uint16_t *)&TxFrame2[TCP_DATA_OFS + 2] = SWAPB(MAX_TCP_RX_DATA_SIZE);// max. length of TCP-data we accept
    *(uint16_t *)&TxFrame2[TCP_CHKSUM_OFS] = CalcChecksum(&TxFrame2[TCP_SRCPORT_OFS], TCP_HEADER_SIZE + TCP_OPT_MSS_SIZE, 1);
    TxFrame2Size = ETH_HEADER_SIZE + IP_HEADER_SIZE + TCP_HEADER_SIZE + TCP_OPT_MSS_SIZE;
  }
  else
  {
    *(uint16_t *)&TxFrame2[TCP_DATA_CODE_OFS] = SWAPB(0x5000 | TCPCode);   // TCP header length = 20
    *(uint16_t *)&TxFrame2[TCP_CHKSUM_OFS] = CalcChecksum(&TxFrame2[TCP_SRCPORT_OFS], TCP_HEADER_SIZE, 1);
    TxFrame2Size = ETH_HEADER_SIZE + IP_HEADER_SIZE + TCP_HEADER_SIZE;
  }

  TransmitControl |= SEND_FRAME2;
}

// easyWEB internal function
// prepares the TxFrame1-buffer to send a payload-packet
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void PrepareTCP_DATA_FRAME(void)
{
  // Ethernet
  memcpy(&TxFrame1[ETH_DA_OFS], &RemoteMAC, 6);
  memcpy(&TxFrame1[ETH_SA_OFS], &MyMAC, 6);
  *(uint16_t *)&TxFrame1[ETH_TYPE_OFS] = SWAPB(FRAME_IP);

  // IP
  *(uint16_t *)&TxFrame1[IP_VER_IHL_TOS_OFS] = SWAPB(IP_VER_IHL | IP_TOS_D);
  WriteWBE(&TxFrame1[IP_TOTAL_LENGTH_OFS], IP_HEADER_SIZE + TCP_HEADER_SIZE + TCPTxDataCount);
  *(uint16_t *)&TxFrame1[IP_IDENT_OFS] = 0;
  *(uint16_t *)&TxFrame1[IP_FLAGS_FRAG_OFS] = 0;
  *(uint16_t *)&TxFrame1[IP_TTL_PROT_OFS] = SWAPB((DEFAULT_TTL << 8) | PROT_TCP);
  *(uint16_t *)&TxFrame1[IP_HEAD_CHKSUM_OFS] = 0;
  memcpy(&TxFrame1[IP_SOURCE_OFS], &MyIP, 4);
  memcpy(&TxFrame1[IP_DESTINATION_OFS], &RemoteIP, 4);
  *(uint16_t *)&TxFrame1[IP_HEAD_CHKSUM_OFS] = CalcChecksum(&TxFrame1[IP_VER_IHL_TOS_OFS], IP_HEADER_SIZE, 0);

  // TCP
  WriteWBE(&TxFrame1[TCP_SRCPORT_OFS], TCPLocalPort);
  WriteWBE(&TxFrame1[TCP_DESTPORT_OFS], TCPRemotePort);

  WriteDWBE(&TxFrame1[TCP_SEQNR_OFS], TCPSeqNr);
  WriteDWBE(&TxFrame1[TCP_ACKNR_OFS], TCPAckNr);
  *(uint16_t *)&TxFrame1[TCP_DATA_CODE_OFS] = SWAPB(0x5000 | TCP_CODE_ACK);   // TCP header length = 20
  *(uint16_t *)&TxFrame1[TCP_WINDOW_OFS] = SWAPB(MAX_TCP_RX_DATA_SIZE);       // data bytes to accept
  *(uint16_t *)&TxFrame1[TCP_CHKSUM_OFS] = 0;
  *(uint16_t *)&TxFrame1[TCP_URGENT_OFS] = 0;
  *(uint16_t *)&TxFrame1[TCP_CHKSUM_OFS] = CalcChecksum(&TxFrame1[TCP_SRCPORT_OFS], TCP_HEADER_SIZE + TCPTxDataCount, 1);
}

// easyWEB internal function
// calculates the TCP/IP checksum. if 'IsTCP != 0', the TCP pseudo-header
// will be included.
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
uint16_t CalcChecksum(void *Start, uint16_t Count, uint8_t IsTCP)
{
  unsigned long Sum = 0;
  uint16_t * piStart;                        // Keil: Pointer added to correct expression

  if (IsTCP) {                                   // if we've a TCP frame...
    Sum += MyIP[0];                              // ...include TCP pseudo-header
    Sum += MyIP[1];
    Sum += RemoteIP[0];
    Sum += RemoteIP[1];
    Sum += SwapBytes(Count);                     // TCP header length plus data length
    Sum += SWAPB(PROT_TCP);
  }

  piStart = Start;                               // Keil: Line added
  while (Count > 1) {                            // sum words
//  Sum += *((uint16_t *)Start)++;		     // Keil: Line replaced with following line
    Sum += *piStart++;
    Count -= 2;
  }

  if (Count)                                     // add left-over byte, if any
//  Sum += *(uint8_t *)Start; 	         // Keil: Line replaced with following line
    Sum += *(uint8_t *)piStart;

  while (Sum >> 16)                              // fold 32-bit sum to 16 bits
    Sum = (Sum & 0xFFFF) + (Sum >> 16);

  return ~Sum;
}

// easyWEB internal function
// starts the timer as a retry-timer (used for retransmission-timeout)
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void TCPStartRetryTimer(void)
{
  TCPTimer = 0;
  RetryCounter = MAX_RETRYS;
  TCPFlags |= TCP_TIMER_RUNNING;
  TCPFlags |= TIMER_TYPE_RETRY;
}

// easyWEB internal function
// starts the timer as a 'TIME_WAIT'-timer (used to finish a TCP-session)
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void TCPStartTimeWaitTimer(void)
{
  TCPTimer = 0;
  TCPFlags |= TCP_TIMER_RUNNING;
  TCPFlags &= ~TIMER_TYPE_RETRY;
}

// easyWEB internal function
// restarts the timer
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void TCPRestartTimer(void)
{
  TCPTimer = 0;
}

// easyWEB internal function
// stopps the timer
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void TCPStopTimer(void)
{
  TCPFlags &= ~TCP_TIMER_RUNNING;
}

// easyWEB internal function
// if a retransmission-timeout occured, check which packet
// to resend.
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void TCPHandleRetransmission(void)
{
  switch (LastFrameSent)
  {
    case ARP_REQUEST :       { PrepareARP_REQUEST(); break; }
    case TCP_SYN_FRAME :     { PrepareTCP_FRAME(TCP_CODE_SYN); break; }
    case TCP_SYN_ACK_FRAME : { PrepareTCP_FRAME(TCP_CODE_SYN | TCP_CODE_ACK); break; }
    case TCP_FIN_FRAME :     { PrepareTCP_FRAME(TCP_CODE_FIN | TCP_CODE_ACK); break; }
    case TCP_DATA_FRAME :    { TransmitControl |= SEND_FRAME1; break; }
  }
}

// easyWEB internal function
// if all retransmissions failed, close connection and indicate an error
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void TCPHandleTimeout(void)
{
  TCPStateMachine = CLOSED;

  if ((TCPFlags & (TCP_ACTIVE_OPEN | IP_ADDR_RESOLVED)) == TCP_ACTIVE_OPEN)
    SocketStatus = SOCK_ERR_ARP_TIMEOUT;         // indicate an error to user
  else
    SocketStatus = SOCK_ERR_TCP_TIMEOUT;

  TCPFlags = 0;                                  // clear all flags
}


/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/

// easyWEB internal function
// function executed every 0.210s by the CPU. used for the
// inital sequence number generator (ISN) and the TCP-timer
//#if defined ( __CC_ARM   ) /*------------------RealView Compiler -----------------*/
//void TCPClockHandler(void) __irq
//#else
void TIMER0_IRQHandler(void)
//#endif

{
  ISNGenHigh++;                                  // upper 16 bits of initial sequence number
  TCPTimer++;                                    // timer for retransmissions
  LPC_TIM0->IR        = 1;                               // Clear interrupt flag
    /* Ack and update VIC Vector Addr */
//    VIC_Ack();
}

// easyWEB internal function
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
uint8_t* GetFrame1Buffer(void)
{
    return ((uint8_t *)_TxFrame1 + ETH_HEADER_SIZE + IP_HEADER_SIZE + TCP_HEADER_SIZE);
}
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
uint8_t* GetRxBuffer(void)
{
    return ((uint8_t *)_RxTCPBuffer);
}

// transfers the contents of 'TxFrame1'-Buffer to the EMAC
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void SendFrame1(void)
{
  SendFrame(TxFrame1, TxFrame1Size);
}

// easyWEB internal function
// transfers the contents of 'TxFrame2'-Buffer to the EMAC
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void SendFrame2(void)
{
  SendFrame(TxFrame2, TxFrame2Size);
}

// easyWEB internal function
// help function to write a WORD in big-endian byte-order
// to MCU-memory
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void WriteWBE(uint8_t *Add, uint16_t Data)
{
  *Add++ = Data >> 8;
  *Add = (char)Data;
}

// easyWEB internal function
// help function to write a DWORD in big-endian byte-order
// to MCU-memory
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
void WriteDWBE(uint8_t *Add, unsigned long Data)
{
  *Add++ = Data >> 24;
  *Add++ = Data >> 16;
  *Add++ = Data >> 8;
  *Add = (char)Data;
}

// easyWEB internal function
// help function to swap the byte order of a WORD
/*********************************************************************//**
 * @brief		
 * @param[in]	
 * @return		
 **********************************************************************/
uint16_t SwapBytes(uint16_t Data)
{
  return (Data >> 8) | (Data << 8);
}

