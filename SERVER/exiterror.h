#ifndef _EXITERROR_H_INCLUDED_
#define _EXITERROR_H_INCLUDED_


/*/////////////////////// Constantes de exiterror - BEGIN - /////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/

#define ERROR_SUCESS							0
#define ERROR_INVALID_OPTION					1
#define ERROR_MISSING_REQUIRED_PARAMETER		2
#define ERROR_INVALID_NUM_VALUE					3
#define ERROR_ONLY_ONE_DAEMON					4
#define ERROR_ONLY_ONE_PORT                     5
#define ERROR_ONLY_ONE_IP                     	6
#define ERROR_ONLY_ONE_PASSWORD                 7
#define ERROR_ONLY_ONE_LOG_LEVEL                8
#define ERROR_ONLY_ONE_SINGLE	                9
#define ERROR_ONLY_ONE_USER						10
#define ERROR_ONLY_ONE_COMMAND					11
#define ERROR_ONLY_ONE_CONFIG_FILE				12
#define ERROR_BAD_PORT_NUMBER					13
#define ERROR_PORT_NUMBER_OUT_OF_RANGE			14
#define ERROR_BAD_LOG_LEVEL_NUMBER              15
#define ERROR_LOG_LEVEL_OUT_OF_RANGE            16
#define ERROR_INVALID_USER_GROUP                17
#define ERROR_NOT_VALID_IP                      18
#define ERROR_NOT_VALID_DEFAULT_IP_PORT			19
#define ERROR_NON_OPTION_ARGUMENT_FOUND			20
#define ERROR_OPTION_VERSION_NOT_ALONE			21
#define ERROR_OPTION_HELP_NOT_ALONE				22
#define ERROR_INVALID_COMMAND_LINE				23


#define ERROR_ALLOCATING_MEMORY					30
#define ERROR_CLOSING_STANDARD_FILE_DESCRIPTORS 31
#define ERROR_SETTING_LOCALE					32


#define ERROR_CONFIG_FILE_UNKNOWN				35
#define ERROR_CONFIG_FILE_READING				36
#define ERROR_CONFIG_FILE_PARSING				37


#define ERROR_FORKING					        40
#define ERROR_CREATING_NEW_SID                  41
#define ERROR_SETTING_WORKING_DIRECTORY         42
#define ERROR_DROPPING_ROOT_PRIVILEGES          43


#define ERROR_CLOSING_CLIENT_SOCKET             50
#define ERROR_CLOSING_LISTENING_SOCKET          51
#define ERROR_GETTING_NET_BYTE_ORDER_IP_ADDRESS 52
#define ERROR_BINDING_TO_IP_ADDRESS             53
#define ERROR_LISTENING                         54
#define ERROR_ACCEPTING_NEW_CONNECTION          55
#define ERROR_SETTING_SOCKET_OPTIONS			56
#define ERROR_SETTING_SOCKET_TIME_OUTS			57


#define ERROR_FORKING_FOR_NEW_CLIENT            60


#define ERROR_PROMOTING_SOCKET_TO_FILE			65
#define ERROR_READING_MAGIC_WORD				66
#define ERROR_BAD_MAGIC_WORD					67
#define ERROR_WRITING_COOKIE                 	68
#define ERROR_READING_HASH						69
#define ERROR_WRITING_ACCEPT_MSG				70
#define ERROR_READING_EVENT						71
#define ERROR_EVENT_LINE_TOO_LONG				72

#define ERROR_GENERATING_PASSWORD_COOKIE		80
#define ERROR_CALCULATING_HASH					81
#define ERROR_HASH_DOES_NOT_MATCH				82


#define ERROR_EXECUTING_COMMAND					85


#define ERROR_INTERRUPTED						253
#define ERROR_SEGMENTATION_FAULT				254
#define ERROR_UNEXPECTED						255

/* =================== */


#define LL_MIN 1
#define LL_ERROR 1
#define LL_INFO 2
#define LL_DEBUG 3
#define LL_MAX 3

#define LOGLEVELNAME(fac) ((fac) == tll_debug)? STR_DEBUG : (((fac) == tll_info)? STR_INFO : STR_ERROR)

#define TLOGLEVELTOACTUALLOGLEVEL(level) ((level) == tll_debug)? LOG_DEBUG : (((level) == tll_info)? LOG_INFO : LOG_ERR)

/*///////////////////////// Constantes de exiterror - END - /////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/


typedef enum TLogLevel {tll_error = LL_ERROR, tll_info = LL_INFO , tll_debug = LL_DEBUG} TLogLevel;

/*/////////////////// Variables globales que se exportan - BEGIN - //////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/

const char* option_with_error;
int config_file_line;

/*/////////////////// Variables globales que se exportan - END - ////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/


/*////////////////////////// Funciones que se exportan - BEGIN - ////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/

bool Start_log();

bool End_log();

void Logit(TLogLevel, const char*, const int, const char*, const char*);

void Vlogit(TLogLevel, const char*, ...);

void Set_actual_log_level(const TLogLevel, const bool);

void Messageerror(const int, const int, const bool);

void Messagewarning(const int, const int, const bool);

void Messagedirect(const TLogLevel, const char*);

/*////////////////////////// Funciones que se exportan - END - //////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/

#endif /* _EXITERROR_H_INCLUDED_ */
