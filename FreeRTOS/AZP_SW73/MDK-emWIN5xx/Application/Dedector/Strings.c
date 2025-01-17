#include "Strings.h"

static char const *TurkishStrs[APP_STR_COUNT] = {
	"TÜRKÇE",
	// SCREEN NAMEs //
	"BALANS",
	"GENEL ARAMA",
	"MİNERALLİ BÖLGE",
	"OTOMATİK ARAMA",
	"DERİNLİK HESAPLAMA",
	"SİSTEM AYARLARI",
	"METAL ANALİZİ",
	"PİL SEVİYESİ KRİTİK SEVİYEYE DÜŞMÜŞTÜR",
	"PİL BİTMİSTİR. OTOMATİK KAPANMA BASLATILIYOR...",
	// BALANS //
	"BASLAMAK İÇİN ONAY BUTONUNA BASINIZ",
	"AYAR YAPILIYOR. LÜTFEN BEKLEYNİZ",
	"BALANS AYARI YAPILDI",
	"MİNERAL DEGERİ",
	"BALANS AYARI YAPILAMADI",
	"LÜTFEN TEKRAR DENEYİNİZ",
	// STD SEARCH //
	"BOŞLUK",
	"MİNERAL",
	"METAL",
	// OTO SEARCH //
	"DEĞERSİZ",
	"DEĞERLİ",
	"ALTIN",
	// DEPTH CALCULATION //
	"DERİNLİK İÇİN ÇAP SEÇİNİZ",
	"EN",
	"BOY",
	"BULUNAN OBJENİN TAHMİNİ DERİNLİĞİ",
	// SYS SETTINGs //
	"SES AYARI",
	"DİL SEÇİMİ",
	"EKRAN PARLAKLIĞI",
	"DEĞERSİZ AYARI",
	"HASSASİYET",
	"FABRİKA AYARLARI",
	"AKTİF",
	"PASİF",
	"FABRİKA AYARLARINA DONMEK İSTİYOR MUSUNUZ?",
	"EVET",
	"HAYIR",
	// LOADING //
	"CİHAZ MODU SEÇİMİ",
	"METAL DEDEKTÖRÜ",
	"ALAN TARAMA",
	"SİSTEM YÜKLENİYOR LÜTFEN BEKLEYİNİZ",
	// AT MENU //
	"OTOMATİK FREKANS",
	"MANUEL FREKANS",
	"MESAFE AYARI",
	"DİL AYARI",
	"EKRAN PARLAKLIĞI",
	"SES AYARI",
	"METRE",
	// AT AUTO FREQ // 
	"UZAK MESAFE ALTIN",
	"YAKIN MESAFE ALTIN",
	"SU",
	"BOŞLUK",
	"TÜM METALLER",
	// AT LOADING // 
	"ALAN TARAMA YÜKLENİYOR",
	"ALTIN: UZUN MESAFE",
	"ALTIN: KISA MESAFE",
	"SU",
	"BOŞLUK",
	"TÜM METALLER",
	// AT MAN FREQ //
	"KHz",
	// AT RADAR // 
	"ALAN TARAMA AKTİF",
	// AT RADAR // 
	"KISA MESAFE",
	"ORTA MESAFE",
	"UZUN MESAFE",
	"MAKSİMUM",	
	// AZP STRINGs // 
	"BÜTÜN METALLER",
	"AYRIM MODU",
	"OTOMATİK TOPRAK AYARI",
	"MANUEL TOPRAK AYARI",
	"TOPRAK DEĞERİ",
	"AÇIK",
	"KAPALI",
	// A5P STRINGs // 
	"HEDEF BULUNDU"
};

static char const *EnglishStrs[APP_STR_COUNT] = {
	"ENGLISH",
	// SCREEN NAMEs //
	"GROUND BALANCE",
	"STANDARD SEARCH",
	"MINERALIZED AREA",
	"AUTOMATIC SEARCH",
	"DEPTH CALCULATION",
	"SYSTEM SETTINGS",
	"METAL ANALYSIS",
	"BATTERY LEVEL IS CRITICAL",
	"BATTERY IS EMPTY. AUTOMATIC POWER OFF IS ACTIVATED",
	// BALANS //
	"PRESS CONFIRM TO START",
	"PROCESSING, PLEASE WAIT",
	"GROUND BALANCE COMPLETED",
	"GROUND ID",
	"GROUND BALANCE FAILED",
	"PLEASE TRY AGAIN",
	// STD SEARCH //
	"CAVITY",
	"MINERAL",
	"METAL",
	// OTO SEARCH //
	"FERROS",
	"NONFERROS",
	"GOLD",
	// DEPTH CALCULATION //
	"SELECT TARGET SIZE FOR CALCULATION",
	"TARGET WIDTH",
	"TARGET HEIGHT",
	"ESTIMATED TARGET DEPTH",
	// SYS SETTINGs //
	"VOLUME",
	"LANGUAGE",
	"LCD BRIGHTNESS",
	"FERROS ELIMINATION",
	"SENSITIVITY",
	"FACTORY SETTINGS",
	"ENABLE",
	"DISABLE",
	"SURE TO RESET TO FACTORY SETTINGS?",
	"YES",
	"NO",
	// LOADING SCREEN //
	"DEVICE MODE SELECTION",
	"METAL DETECTOR",
	"FIELD SCANNER",
	"LOADING PLEASE WAIT",
	// AT MENU // 
	"AUTOMATIC FREQUENCY",
  "MANUAL FREQUENCY",
	"RANGE",
	"LANGUAGE",
	"BRIGHTNESS",
	"VOLUME",
	"METER",
	// AT AUTO FREQ // 
	"LONG RANGE GOLD",
	"SHORT RANGE GOLD",
	"WATER",
	"CAVITY",
	"ALL METALS",
	// AT LOADING // 
	"LOADING FIELD SCANNER",
	"GOLD: LONG RANGE",
	"GOLD: SHORT RANGE",
	"WATER",
	"CAVITY",
	"ALL METALS",
	// AT MAN FREQ //
	"KHz",
	// AT RADAR // 
	"SEARCHING THE AREA",
	// AT RADAR // 
	"SHORT RANGE",
	"MID RANGE",
	"LONG RANGE",
	"MAXIMUM",
	// AZP STRINGs // 
	"ALL METALS",
	"GRAPHICAL DISCRIMINATION",
	"AUTO GROUND BALANCE",
	"MANUAL GROUND BALANCE",
	"MINERALIZATION",
	"ENABLED",
	"DISABLED",
	// A5P STRINGs // 
	"TARGET FOUND"
};

static char const *ArabicStrs[APP_STR_COUNT] = {
	"عربي",
	// SCREEN NAMEs //
	"موازنة",
	"بحث قياسي",
	"منطقة معدنية",
	"بحث تلقائي",
	"حساب العمق",
	"عيارات المنظومة",
	"تحليل المعدن",
	"مستوى البطارية منخفض الى المستوى الحرج",
	"البطارية منتهية.. يتم البدء بالاغلاق التلقائي"
	// BALANS //
	"للبدء اضغط على زر الموافقة",
	"جاري تنظيم العيارات. يرجى الانتظار",
	"تمت الموازنة. قيمة المعادن في التربة : ",
	"التربة معرف",
	"لم تتم الموازنة. يرجى اعادة المحاولة",
	"حاول مرة اخرى",
	// STD SEARCH //
	"فراغ",
	"معدني",
	"معدن",
	// OTO SEARCH //
	"بدون قيمة",
	"ذو قيمة",
	"ذهب",
	// DEPTH CALCULATION //
	"لتحديد العمق يرجى اختيار القطر",
	"العرض",
	"الطول",
	"العمق التخميني للجسم الموجود : ",
	// SYS SETTINGs //
	"ضبط الصوت",
	"ضبط اللغة",
	"لمعان الشاشة",
	"عيار بدون قيمة",
	"الحساسية",
	"عيارات المصنع",
	"فعال",
	"غير فعال",
	"هل ترغب بالعودة الى عيارات المصنع؟ ",
	"نعم",
	"كلا",
	// LOADING SCREEN //
	"نوع الجهاز اختيار",
	"كاشف المعادن",
	"الماسح الضوئي الحقل",
	"جاري التحميل الرجاءالانتظار",
	// AT MENU //
	"تردد أوتوماتيكي",
	"تردد اليدوي",
	"نطاق",
	"لغة",
	"سطوع",
	"الصوت",
	"متر",
	// AT AUTO FREQUENCY // 
	"الذهب: بعيدة المدى",
	"الذهب: قصيرة المدى",
	"ماء",
	"تجويف",
	"جميع المعادن",
	// AT LOADING //
	"الماسح الضوئي تحميل الملف",
	"الذهب: بعيدة المدى",
	"الذهب: قصيرة المدى",
	"ماء",
	"تجويف",
	"جميع المعادن",
	// AT MAN FREQ //
	"كيلوهرتز",
	// AT RADAR //
	"منطقة البحث",
	// AT RADAR // 
	"مدى قصير",
	"مدى متوسط",
	"بعيد المدى",
	"أقصى مدى",
	// AZP STRINGs // 
	"كل المعادن",
	"التمييز الجرافيكي",
	"التوازن الأرضي التلقائي",
	"الميزان الارضي",
	"تمعدن",
	"نشيط",
	"تعطيل",
	// A5P STRINGs // 
	"الهدف وجدت"
};


static char const *GermanStrs[APP_STR_COUNT] = {
	"GERMAN",
	// SCREEN NAMEs //
	"GRUNDBALANZ",
	"STANDARDSUCHE",
	"MINERALISIERUNG D-BEREICH",
	"AUTOMATISCHE SUCHE",
	"TIEFENBERECHNUNG",
	"SYSTEME INSTELLUNGEN",
	"METALLANALYSE",
	"BATTERIE STAND IST KRITISCH",
	"BATTERIE IST LEER. AUTOMATISCHE AUSSCHALTUNG AKTIVIERT",
	// BALANCE //
	"ZUM STARTEN AUF BESTAETIGUNG DRÜCKEN",
	"WIRD VERARBEITET. BITTE WARTEN.",
	"GRUNDBALANZ ABGESCHLOSSEN.",
	"GRUND ID",
	"GRUNDBALANZ GESCHEITERT.",
	"BITTE ERNEUT VERSUCHEN",
	// STD SEARCH //
	"KAVITAET",
	"MINERAL",
	"METALL",
	// OTO SEARCH //
	"FERROS",
	"NICHT FERROS",
	"GOLD",
	// DEPTH CALCULATION //
	"FÜR DIE TIEFE EIN ZIELAUSMASS AUSSUCHEN",
	"ZIELBREITE",
	"ZIELHÖHE",
	"EINGESCHAETZTE ZIELTIEFE",
	// SYS SETTINGs //
	"LAUTSTAERKENEINSTELLUNG",
	"SPRACHE",
	"LCD HELLIGKEIT",
	"FERROS ELIMINATION",
	"SENSIBILITAET",
	"WERKEINSTELLUNGEN",
	"AKTIV",
	"NICHT AKTIV",
	"SIND SIE SICHER, DASS SIE AUF WERKEINSTELLUNGEN ZURÜCKSETZEN WOLLEN?",
	"JA",
	"NEIN",
	// LOADING SCREEN //
	"DEVICE TYPE AUSWAHL",
	"METALL DETEKTOR",
	"FELD SCANNER",
	"LADEN, BITTE WARTEN",
	// AT MENU //
	"AUTOMATISCHE FREQUENZ",
	"MANUELLE FREQUENZEINSTELLUNG",
	"Angebot",
	"Sprache",
	"Helligkeit",
	"Volumen",
	"Meter",
	// AT AUTO FREQ // 
	"gold: lange reichweite",
	"gold: kurze bereich",
	"wasser",
	"hohlraum",
	"alle metallen",
	// AT LOADING //
	"laden von datei-scanner",
	"gold: lange reichweite",
	"gold: kurze bereich",
	"wasser",
	"hohlraum",
	"alle metallen",
	// AT MAN FREQ //
	"KHz",
	// AT RADAR // 
	"Suchbereich",
	// AT RADAR // 
	"KURZE REICHWEITE",
	"MITTELBEREICH",
	"LANGSTRECKEN",
	"MAXIMALER BEREICH",
	// AZP STRINGs // 
	"ALLE METALLE",
	"GRAPHISCHE DISKRIMINIERUNG",
	"automatischer Bodenausgleich",
	"manuelle Bodenbalance",
	"MINERALISIERUNG",
	"aktiviert",
	"behindert",
	// A5P STRINGs // 
	"Ziel gefunden"
};

char const *SpanishStrs[APP_STR_COUNT] = {
	"ESPAÑOL",
	// SCREEN NAMEs //
	"BALANCE DE SUELO",
	"BÚSQUEDA ESTÁNDAR",
	"ÁREA MINERALIZADA",
	"BÚSQUEDA AUTOMÁTICA",
	"CÁLCULO DE PROFUNDIDAD",
	"AJUSTES DEL SISTEMA",
	"ANÁLISIS DE METAL",
	"LA BATERÍA HA ALCANZADO UN NIVEL CRÍTICO.",
	"LA BATERÍA ESTÁ VACÍA... SE ACTIVA EL AUTOAPAGADO",
	// BALANS //
	"PULSE CONFIRMAR PARA INICIAR",
	"PROCESANDO. POR FAVOR ESPERE",
	"BALANCE DE SUELO COMPLETADO",
	"VALOR DEL MINERAL DEL SUELO",
	"BALANCE DE SUELO FALLADO",
	"INTÉNTELO DE NUEVO",
	// STD SEARCH //
	"CAVIDAD",
	"MINERAL",
	"METAL",
	// OTO SEARCH //
	"FERROSO",
	"NO FERROSO",
	"ORO",
	// DEPTH CALCULATION //
	"SELECCIONE DIÁMETRO PARA LA PROFUNDIDAD",
	"ANCHO",
	"LARGO",
	"PROFUNDIDAD ESTIMADA DEL OBJETO DETECTADO",
	// SYS SETTINGs //
	"VOLUMEN",
	"IDIOMA",
	"BRILLO DE LA PANTALLA",
	"AJUSTE DE FERROSO",
	"SENSIBILIDAD",
	"CONFIGURACIÓN DE FÁBRICA",
	"ACTIVAR",
	"DESACTIVAR",
	"¿DESEA REGRESAR A LA CONFIGURACIÓN DE FÁBRICA?",
	"SÍ",
	"NO",
	// LOADING SCREEN //
	"SELECCIÓN DE DISPOSITIVO DE TIPO",
	"DETECTOR DE METALES",
	"ESCÁNER DE CAMPO",
	"CARGANDO POR FAVOR ESPERE",
	// AT MENU //
	"automático de frecuencia",
	"manual de frecuencia",
	"distancia",
	"idioma",
	"brillo",
	"volumen",
	"metro",
	// AT AUTO FREQ // 
    "oro: de largo alcance",
    "oro: de corto alcance",
    "agua",
    "cavidad",
    "todos los metales",
	// AT LOADING //
	"examinador de archivos de carga",
	"oro: de largo alcance",
	"oro: de corto alcance",
	"agua",
	"cavidad",
	"todos los metales",
	// AT MAN FREQ //
	"KHz",
	// AT RADAR // 
	"área de búsqueda",
	// AT RADAR // 
	"CORTO ALCANCE",
	"RANGO MEDIO",
	"DE LARGO ALCANCE",
	"RANGO MAXIMO",	
	// AZP STRINGs // 
	"TODOS LOS METALES",
	"DISCRIMINACIÓN GRAFICA",
	"balance de suelo automático",
	"balance de tierra manual",
	"MINERALIZACIÓN",
	"habilitado",
	"discapacitado",
	// A5P STRINGs // 
	"objetivo encontrado"
};

static char const *PersianStrs[APP_STR_COUNT] = {
	"فارسی",
	// SCREEN NAMEs //
	"بالانس",
	"جستجوی استاندارد",
	"منطقه مینرالیزه",
	"جستجوی اتوماتیک",
	"محاسبه عمق",
	"تنظیمات سیستم",
	"آنالیز فلز",
	"سطح باتری به سطح  بحرانی رسیده است.",
	"باتری تمام شده است ...اتوماتیک بسته می شود.",
	// BALANS //
	"برای شروع دکمه تائید را فشار دهید.",
	"در حال تنظیم می باشد. لطفا صبر کنید.",
	"بالانس انجام شد.ارزش معدنی خاک",
	"شناسه خاک",
	"بالانس انجام نشد. لطفا مجددا امتحان نمائید.",
	"لطفا دوباره تلاش کنید",
	// STD SEARCH //
	"فضای خالی",
	"معدنی",
	"فلز",
	// OTO SEARCH //
	"بی ارزش",
	"با ارزش",
	"طلا",
	// DEPTH CALCULATION //
	"برای عمق، قطر انتخاب نمائید.",
	"عرض",
	"طول",
	"عمق تخمینی مورد بدست آمده:",
	// SYS SETTINGs //
	"تنظیمات صدا",
	"تنظیمات زبان",
	"روشنایی صفحه نمایش",
	"تنظیمات بی ارزش",
	"حساسیت",
	"تنظیمات کارخانه",
	"فعال",
	"غیرفعال",
	"آیا میخواهید به تنظیمات کارخانه بازگردید؟",
	"بله",
	"نه",
	// LOADING SCREEN //
	"انتخاب نوع دستگاه",
	"فلزیاب",
	"رشته اسکنر",
	"در حال بارگذاری لطفا صبر کنید",
		// AT MENU //
	"فرکانس به صورت خودکار",
	"فرکانس دستی",
	"محدوده",
	"زبان",
	"روشنایی",
	"حجم",
	"متر",
	// AT AUTO FREQ // 
	"طلا: دوربرد",
	"طلا: برد کوتاه",
	"اب",
	"حفره",
	"همه فلزات",
	// AT LOADING //
	"اسکنر فایل بارگذاری",
	"طلا: دوربرد",
	"طلا: برد کوتاه",
	"اب",
	"حفره",
	"همه فلزات",
	// AT MAN FREQ //
	"کیلوهرتز",
	// AT RADAR // 
	"منطقه جستجو",
	// AT RADAR // 
	"برد کوتاه",
	"گسترهی میانی",
	"دوربرد",
	"حداکثر برد",	
	// AZP STRINGs // 
	"تمام فلزات",
	"تبعیض نژادی",
	"خودکار تعادل زمین",
	"تعادل زمین منو",
	"معدنی شدن",
	"فعال",
	"قابل اعتماد",
	// A5P STRINGs // 
	"هدف یافت شد"
};


static char const *RussStrs[APP_STR_COUNT] = { 
	"РУССКИЙ",
	// SCREEN NAMEs //
	"БАЛАНС",
	"СТАНДАРТНЫЙ ПОИСК",
	"МИНЕРАЛЬНАЯ ЗОНА",
	"АВТОМАТИЧЕСКИЙ ПОИСК",
	"РАСЧЕТ ГЛУБИНЫ",
	"НАСТРОЙКИ СИСТЕМЫ",
	"АНАЛИЗ МЕТАЛЛА",
	"ЗАРЯДКА БАТАРЕИ НА КРИТИЧЕСКОМ УРОВНЕ",
	"БАТАРЕЯ РАЗРЯДИЛАСЬ. ВЫПОЛНЯЕТСЯ АВТОМАТИЧЕСКОЕ ВЫКЛЮЧЕНИЕ",
	// BALANS //
	"НАЖМИТЕ НА КНОПКУ ПОДТВЕРЖДЕНИЯ ДЛЯ НАЧАЛА ОПЕРАЦИИ",
	"ВЫПОЛНЯЕТСЯ НАСТРОЙКА. ПОЖАЛУЙСТА ПОДОЖДИТЕ",
	"БАЛАНСИРОВКА ЗАВЕРШЕНА",
	"МИНЕРАЛЬНОЕ СОДЕРЖАНИЕ ГРУНТА",
	"БАЛАНСИРОВКА НЕ ВЫПОЛНЕНА",
	"ПОПРОБУЙТЕ ЕЩЕ РАЗ",
	// STD SEARCH //
	"ПУСТОТА",
	"МИНЕРАЛ",
	"МЕТАЛЛ",
	// OTO SEARCH //
	"НЕ ДРАГОЦЕННЫЙ",
	"ДРАГОЦЕННЫЙ",
	"ЗОЛОТО",
	// DEPTH CALCULATION //
	"ВЫБЕРИТЕ ДИАМЕТР ДЛЯ ГЛУБИНЫ",
	"ШИРИНА",
	"ДЛИНА",
	"ПРИБЛИЗИТЕЛЬНАЯ ГЛУБИНА НАЙДЕННОГО ОБЪЕКТА",
	// SYS SETTINGs //
	"НАСТРОЙКА ЗВУКА",
	"НАСТРОЙКА ЯЗЫКА",
	"ЯРКОСТЬ ЭКРАНА",
	"НАСТРОЙКА НЕДРАГОЦЕННЫЙ",
	"ТОЧНОСТЬ",
	"ЗАВОДСКИЕ НАСТРОЙКИ",
	"АКТИВНЫЙ",
	"НЕАКТИВНЫЙ",
	"ВЫ ХОТИТЕ ВЕРНУТЬСЯ К ЗАВОДСКИМ НАСТРОЙКАМ?",
	"ДА",
	"НЕТ",
	// LOADING SCREEN //
	"УСТРОЙСТВО ТИП ВЫБОР",
	"МЕТАЛЛОИСКАТЕЛЬ",
	"Область техники СКАНЕР",
	"ЗАГРУЗКА, ПОЖАЛУЙСТА ПОДОЖДИТЕ",
		// AT MENU //
	"автоматическая частота",
	"ручная частота",
	"ассортимент",
	"язык",
	"яркость",
	"объем",
	"метр",
	// AT AUTO FREQ // 
	"золото: большой дальности",
	"ЗОЛОТО: БЛИЖНЕГО",
	"ВОДЫ",
	"ПОЛОСТЕЙ",
	"ВСЕ МЕТАЛЛЫ",
	// AT LOADING //
	"сканер загрузки файлов",
	"золото: большой дальности",
	"ЗОЛОТО: БЛИЖНЕГО",
	"ВОДЫ",
	"ПОЛОСТЕЙ",
	"ВСЕ МЕТАЛЛЫ",
	// AT MAN FREQ //
	"Кило Гц",
	// AT RADAR // 
	"поиска область",
	// AT RADAR // 
	"НА КОРОТКИЕ РАССТОЯНИЯ",
	"БЛИЖНИЙ RANGE",
	"дальность полета",
	"Самый большой дальности",	
	// AZP STRINGs // 
	"ВСЕ МЕТАЛЛЫ",
	"ГРАФИЧЕСКАЯ ДИСКРИМИНАЦИЯ",
	"автоматический баланс грунта",
	"ручной баланс грунта",
	"МИНЕРАЛИЗАЦИЯ",
	"включен",
	"отключен",
	// A5P STRINGs // 
	"найденная цель"
};

static char const *FrenchStrs[APP_STR_COUNT] = {
	"FRANÇAIS",
	// SCREEN NAMEs // 
	"EQUILIBRE DU SOL",
	"RECHERCHE STANDARD",
	"ZONE MINERALISEE",
	"RECHERCHE AUTOMATIQUE",
	"CALCUL DE PROFONDEUR",
	"REGLAGES DU SYSTEME",
	"ANALYSE DE METAL",
	"LE NIVEAU DE LA BATTERIE EST DESCENDU JUSQU’AU NIVEAU CRITIQUE",
	"LA BATTERIE EST VIDE… L’EXTINCTION AUTOMATIQUE EST ACTIVEE",
	// BALANS //
	"PRESSER SUR LE BOUTON DE CONFIRMATION POUR COMMENCER",
	"REGLAGE EN COURS. VEUILLEZ PATIENTER ",
	"L’EQUILIBRAGE DU SOL EST TERMINE",
	"VALEUR MINERALE DU SOL",
	"L’EQUILIBRAGE DU SOL A ECHOUE",
	"VEUILLEZ RECOMMENCER",
	// STD SEARCH //
	"CAVITE",
	"MINERAL",
	"METAL",
	// OTO SEARCH //
	"SANS VALEUR",
	"DE VALEUR",
	"OR",
	// DEPTH CALCULATION //
	"SÉLECTIONNER LA TAILLE CIBLE",
	"LARGEUR",
	"LONGUEUR",
	"VALEUR ESTIME DE L’OBJET DECOUVERT",
	// SYS SETTINGs //
	"VOLUME",
	"LANGAGE",
	"LUMINOSITE DE L’ECRAN",
	"REGLAGE SANS VALEUR",
	"SENSIBILITE",
	"REGLAGE D’USINE",
	"ACTIF",
	"PASSIF",
	"DESIREZ-VOUS RETOURNER AUX REGLAGES D’USINE",
	"OUI",
	"NON",
	// LOADING SCREEN //
	"TYPE DE DISPOSITIF DE SÉLECTION",
	"DÉTECTEUR DE MÉTAUX",
	"DOMAINE SCANNER",
	"CHARGEMENT, VEUILLEZ PATIENTER",
		// AT MENU //
	"fréquence automatique",
	"fréquence manuelle",
	"gamme",
	"la langue",
	"luminosité",
	"le volume",
	"mètre",
	// AT AUTO FREQ // 
    "or: longue portée",
    "or: courte portée",
    "eau",
    "cavite",
    "Tous les métaux",
	// AT LOADING //
	"analyseur de fichiers de chargement",
	"or: longue portée",
	"or: courte portée",
	"eau",
	"cavite",
	"Tous les métaux",
	// AT MAN FREQ //
	"KHz",
	// AT RADAR // 
	"zone de recherche",
	// AT RADAR // 
	"COURTE PORTÉE",
	"MILIEU DE GAMME",
	"longue portée",
	"GAMME MAXIMUM",
	// AZP STRINGs // 
	"TOUS MÉTAUX",
	"DISCRIMINATION GRAPHIQUE",
	"balance au sol automatique",
	"équilibre manuel du sol",
	"МИНЕРАЛИЗАЦИЯ",
	"activée",
	"désactivée",
	// A5P STRINGs // 
	"cible trouvée"
};

char const **AllLangStrs[LANG_COUNT] = 
{
	TurkishStrs, EnglishStrs, ArabicStrs, \
	GermanStrs, SpanishStrs, PersianStrs, RussStrs, FrenchStrs
};
