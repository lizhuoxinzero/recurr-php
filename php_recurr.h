/**********************************************************\
|                                                          |
| php_recurr.h                                             |
|                                                          |
| RECURR for pecl include file.                            |
|                                                          |
| Code Author:  Never Lee <listarmb@gmail.com>             |
| LastModified: Apr 06, 2015                               |
|                                                          |
\**********************************************************/

#ifndef PHP_RECURR_H
#define PHP_RECURR_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "recurr.h"
#include "datemap.h"

extern zend_module_entry recurr_module_entry;
#define phpext_recurr_ptr &recurr_module_entry

#define PHP_RECURR_MODULE_NAME   "recurr"
#define PHP_RECURR_BUILD_DATE    __DATE__ " " __TIME__
#define PHP_RECURR_VERSION       "0.1.0"
#define PHP_RECURR_AUTHOR        "Never Lee"
#define PHP_RECURR_HOMEPAGE      "https://github.com/neverlee/recurr-pecl"

ZEND_MINIT_FUNCTION(recurr);
ZEND_MSHUTDOWN_FUNCTION(recurr);
ZEND_MINFO_FUNCTION(recurr);

/* declaration of functions to be exported */
ZEND_FUNCTION(RecurrInfo);
ZEND_FUNCTION(recurr_isleap);
ZEND_FUNCTION(recurr_daysin);
ZEND_FUNCTION(recurr_datetime);

PHP_METHOD(Recurr, __construct);
PHP_METHOD(Recurr, setExDates);
PHP_METHOD(Recurr, dump);
PHP_METHOD(Recurr, constraint);

typedef struct {
    zend_object std;
    rule_t rule;
} php_recurr_obj;

static inline php_recurr_obj *php_recurr_obj_from_obj(zend_object *obj) {
	return (php_recurr_obj*)(obj);
}
//#define Z_PHPRECURR_P(zv)  php_recurr_obj_from_obj(Z_OBJ_P((zv)))
#define Z_PHPRECURR_P(zv)  php_recurr_obj_from_obj(zend_object_store_get_object((zv) TSRMLS_CC))


#endif /* ifndef PHP_RECURR_H */
