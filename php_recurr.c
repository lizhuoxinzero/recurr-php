/**********************************************************\
|                                                          |
| php_recurr.c                                             |
|                                                          |
| RECURR for pecl source file.                             |
|                                                          |
| Code Author:  Never Lee <listarmb@gmail.com>             |
| LastModified: Apr 09, 2015                               |
|                                                          |
\**********************************************************/

#include "php_recurr.h"

#include "php.h"

#if defined(_MSC_VER)
#include "win32/php_stdint.h"
#elif defined(__FreeBSD__) && __FreeBSD__ < 5
/* FreeBSD 4 doesn't have stdint.h file */
#include <inttypes.h>
#else
#include <stdint.h>
#endif

#include <sys/types.h> /* This will likely define BYTE_ORDER */


#ifndef BYTE_ORDER
#if (BSD >= 199103)
# include <machine/endian.h>
#else
#if defined(linux) || defined(__linux__)
# include <endian.h>
#else
#define LITTLE_ENDIAN   1234    /* least-significant byte first (vax, pc) */
#define BIG_ENDIAN  4321    /* most-significant byte first (IBM, net) */
#define PDP_ENDIAN  3412    /* LSB first in word, MSW first in long (pdp)*/

#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__) || \
   defined(vax) || defined(ns32000) || defined(sun386) || \
   defined(MIPSEL) || defined(_MIPSEL) || defined(BIT_ZERO_ON_RIGHT) || \
   defined(__alpha__) || defined(__alpha)
#define BYTE_ORDER    LITTLE_ENDIAN
#endif

#if defined(sel) || defined(pyr) || defined(mc68000) || defined(sparc) || \
    defined(is68k) || defined(tahoe) || defined(ibm032) || defined(ibm370) || \
    defined(MIPSEB) || defined(_MIPSEB) || defined(_IBMR2) || defined(DGUX) ||\
    defined(apollo) || defined(__convex__) || defined(_CRAY) || \
    defined(__hppa) || defined(__hp9000) || \
    defined(__hp9000s300) || defined(__hp9000s700) || \
    defined (BIT_ZERO_ON_LEFT) || defined(m68k) || defined(__sparc)
#define BYTE_ORDER  BIG_ENDIAN
#endif
#endif /* linux */
#endif /* BSD */
#endif /* BYTE_ORDER */

#ifndef BYTE_ORDER
#ifdef __BYTE_ORDER
#if defined(__LITTLE_ENDIAN) && defined(__BIG_ENDIAN)
#ifndef eITTLE_ENDIAN
#define LITTLE_ENDIAN __LITTLE_ENDIAN
#endif
#ifndef BIG_ENDIAN
#define BIG_ENDIAN __BIG_ENDIAN
#endif
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define BYTE_ORDER LITTLE_ENDIAN
#else
#define BYTE_ORDER BIG_ENDIAN
#endif
#endif
#endif
#endif

typedef int length_t;
#define __RETURN_STRINGL(s, l) RETURN_STRINGL(s, l, 0)
#define __add_assoc_string(arg, key, str) add_assoc_string(arg, key, str, 1)

#define MX (((z >> 5) ^ (y << 2)) + ((y >> 3) ^ (z << 4))) ^ ((sum ^ y) + (key[(p & 3) ^ e] ^ z))
#define DELTA 0x9e3779b9

ZEND_BEGIN_ARG_INFO_EX(recurr_info_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(recurr_isleap_arginfo, 0, 0, 0)
    ZEND_ARG_INFO(0, year)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(recurr_daysin_arginfo, 0, 0, 0)
    ZEND_ARG_INFO(0, year)
    ZEND_ARG_INFO(0, month)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(recurr_datetime_arginfo, 0, 0, 0)
    ZEND_ARG_INFO(0, stamp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_recurr_construct, 0, 0, 0)
    ZEND_ARG_INFO(0, start)
    ZEND_ARG_INFO(0, end)
    ZEND_ARG_INFO(0, freq)
    ZEND_ARG_INFO(0, interval)
    ZEND_ARG_INFO(0, until)
    ZEND_ARG_INFO(0, lunarflag)
    ZEND_ARG_INFO(0, monthfix)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_recurr_set_exdates, 0, 0, 0)
    ZEND_ARG_INFO(0, stamps)
    ZEND_ARG_INFO(0, byday)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_recurr_dump, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_recurr_constraint, 0, 0, 0)
    ZEND_ARG_INFO(0, before)
    ZEND_ARG_INFO(0, after)
    ZEND_ARG_INFO(0, limit)
ZEND_END_ARG_INFO()


#ifndef PHP_FE_END
#define PHP_FE_END            { NULL, NULL, NULL }
#endif

static zend_object_handlers recurr_object_handlers_recurr;

static zend_object_value recurr_object_new_recurr(zend_class_entry *class_type TSRMLS_DC);
static zend_object_value recurr_object_clone_recurr(zval *this_ptr TSRMLS_DC);
static void recurr_object_free_storage_recurr(zend_object *object);
//static HashTable *recurr_object_get_gc(zval *object, zval **table, int *n);
//

static void rule_constraint(zval *rv, rule_t *r, datetime_t after, datetime_t before, int limit);

/* compiled function list so Zend knows what's in this module */
zend_function_entry recurr_functions[] = {
    ZEND_FE(RecurrInfo, recurr_info_arginfo)
    PHP_FE_END
};

/* compiled module information */
zend_module_entry recurr_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_RECURR_MODULE_NAME,
    recurr_functions,
    ZEND_MINIT(recurr),
    ZEND_MSHUTDOWN(recurr),
    NULL,
    NULL,
    ZEND_MINFO(recurr),
    PHP_RECURR_VERSION,
    STANDARD_MODULE_PROPERTIES
};


/* implement standard "stub" routine to introduce ourselves to Zend */
#if defined(COMPILE_DL_RECURR)
ZEND_GET_MODULE(recurr)
#endif


zend_class_entry *recurr_ce;
zend_class_entry *itime_ce;

static zend_function_entry recurr_method[] = {
    ZEND_ME_MAPPING(isleap, recurr_isleap, recurr_isleap_arginfo, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    ZEND_ME_MAPPING(daysin, recurr_daysin, recurr_daysin_arginfo, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    ZEND_ME_MAPPING(datetime, recurr_datetime, recurr_datetime_arginfo, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)

    PHP_ME(Recurr, __construct, arginfo_recurr_construct, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
    PHP_ME(Recurr, setExDates, arginfo_recurr_set_exdates, ZEND_ACC_PUBLIC)
    PHP_ME(Recurr, dump, arginfo_recurr_dump, ZEND_ACC_PUBLIC)
    PHP_ME(Recurr, constraint, arginfo_recurr_constraint, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

ZEND_MINIT_FUNCTION(recurr) {
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "Recurr", recurr_method);
    ce.create_object = recurr_object_new_recurr;
    recurr_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
    memcpy(&recurr_object_handlers_recurr, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    recurr_object_handlers_recurr.clone_obj = recurr_object_clone_recurr;
    //zend_declare_property_double(yar_client_ce, ZEND_STRL("_protocol"), YAR_CLIENT_PROTOCOL_HTTP, ZEND_ACC_PROTECTED TSRMLS_CC);

    REGISTER_LONG_CONSTANT("FREQ_SECONDLY", SECONDLY   , CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("FREQ_MINUTELY", MINUTELY   , CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("FREQ_HOURLY"  , HOURLY     , CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("FREQ_DAILY"   , DAILY      , CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("FREQ_WEEKLY"  , WEEKLY     , CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("FREQ_MONTHLY" , MONTHLY    , CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("FREQ_YEARLY"  , YEARLY     , CONST_PERSISTENT | CONST_CS);

    REGISTER_LONG_CONSTANT("MFIX_IGNORE"  , Ignore     , CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("MFIX_LASTDAY" , UseLastDay , CONST_PERSISTENT | CONST_CS);

    return SUCCESS;
}

static inline zend_object_value recurr_object_new_recurr_ex(zend_class_entry *class_type, php_recurr_obj **ptr TSRMLS_DC) {
    php_recurr_obj *intern;
    zend_object_value retval;
    intern = ecalloc(1, sizeof(php_recurr_obj));
    memset(intern, 0, sizeof(php_recurr_obj));
    if (ptr) {
        *ptr = intern;
    }

    zend_object_std_init(&intern->std, class_type);
    object_properties_init(&intern->std, class_type);
    retval.handle = zend_objects_store_put(intern, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t)recurr_object_free_storage_recurr, NULL TSRMLS_CC);
    retval.handlers = &recurr_object_handlers_recurr;
    return retval;
}

static zend_object_value recurr_object_new_recurr(zend_class_entry *class_type TSRMLS_DC) /* {{{ */
{
    return recurr_object_new_recurr_ex(class_type, NULL TSRMLS_CC);
}

static zend_object_value recurr_object_clone_recurr(zval *this_ptr TSRMLS_DC) /* {{{ */
{
    php_recurr_obj *old_obj = Z_PHPRECURR_P(this_ptr);
    php_recurr_obj *new_obj = NULL;
    zend_object_value new_ov = recurr_object_new_recurr_ex(old_obj->std.ce, &new_obj TSRMLS_CC);

    zend_objects_clone_members(&new_obj->std, new_ov, &old_obj->std, Z_OBJ_HANDLE_P(this_ptr) TSRMLS_CC);

    memcpy(&(new_obj->rule), &(old_obj->rule), sizeof(rule_t));
    if (old_obj->rule.exdates.dates) {
        new_obj->rule.exdates.dates = emalloc(sizeof(datetime_t)*old_obj->rule.exdates.size);
        memcpy(new_obj->rule.exdates.dates, old_obj->rule.exdates.dates, sizeof(datetime_t)*old_obj->rule.exdates.size);
    }

    return new_ov;
} /* }}} */


static void recurr_object_free_storage_recurr(zend_object *object) /* {{{ */
{
    php_recurr_obj *intern = php_recurr_obj_from_obj(object);

    if (intern->rule.exdates.dates) {
        efree(intern->rule.exdates.dates);
        intern->rule.exdates.dates = NULL;
    }

    zend_object_std_dtor(&intern->std);
} /* }}} */

ZEND_MSHUTDOWN_FUNCTION(recurr) {
    return SUCCESS;
}

ZEND_MINFO_FUNCTION(recurr) {
    php_info_print_table_start();
    php_info_print_table_row(2, "recurr support", "enabled");
    php_info_print_table_row(2, "recurr version", PHP_RECURR_VERSION);
    php_info_print_table_row(2, "recurr author", PHP_RECURR_AUTHOR);
    php_info_print_table_row(2, "recurr homepage", PHP_RECURR_HOMEPAGE);
    php_info_print_table_end();
}

/* {{{ proto RecurrInfo */
ZEND_FUNCTION(RecurrInfo) {
    array_init(return_value);
    __add_assoc_string(return_value, "ext_version", PHP_RECURR_VERSION);
    __add_assoc_string(return_value, "ext_build_date", PHP_RECURR_BUILD_DATE);
    __add_assoc_string(return_value, "ext_author", PHP_RECURR_AUTHOR);
}
/* }}} */

/* {{{ proto Recurr::isLeap(int $year) */
ZEND_FUNCTION(recurr_isleap) {
    long year;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &year) == FAILURE) {
        return;
    }
    RETURN_BOOL(is_leap(year));
}
/* }}} */

/* {{{ proto Recurr::daysIn(int $year, int $month) */
ZEND_FUNCTION(recurr_daysin) {
    long year, month;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &year, &month) == FAILURE) {
        return;
    }
    int days = days_in(month, year);
    RETURN_LONG(days);
}
/* }}} */

/* {{{ proto Recurr::datetime(double $stamp, int $month) */
ZEND_FUNCTION(recurr_datetime) {
    double stamp;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &stamp) == FAILURE) {
        return;
    }
    datetime_t dt = sdt_unix((__int64_t)stamp);
    int year, month, day;
    int hour, minute, second;
    hour = dt % 86400 / 3600;
    minute = dt % 3600 / 60;
    second = dt % 60;
    dt_date(dt, &year, &month, &day);
    array_init(return_value);
    add_assoc_long(return_value, "year", year);
    add_assoc_long(return_value, "month", month);
    add_assoc_long(return_value, "day", day);
    add_assoc_long(return_value, "hour", hour);
    add_assoc_long(return_value, "minute", minute);
    add_assoc_long(return_value, "second", second);
}
/* }}} */


/* {{{ proto Recurr::__construct(...) */
PHP_METHOD(Recurr, __construct)
{
    double start, end = 0, until = 1<32;
    long freq = WEEKLY, interval = 1, monthfix = UseLastDay, lunarflag = Solar;
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d|dlldll", &start, &end, &freq, &interval, &until, &lunarflag, &monthfix)) {
        return;
    }

    if (end < start) {
        end = start;
    }

    php_recurr_obj *pr = Z_PHPRECURR_P(getThis());
    rule_t *rule = &(pr->rule);
    rule->start = sdt_unix((__int64_t)start);
    rule->end = sdt_unix((__int64_t)end);
    rule->freq = freq;
    rule->interval = interval;
    rule->until = sdt_unix((__int64_t)until);
    rule->lunarflag = lunarflag;
    rule->monthfix = monthfix;
}
/* }}} */

/* {{{ proto Recurr::setExDates(...) */
PHP_METHOD(Recurr, setExDates)
{
    php_recurr_obj *pr = Z_PHPRECURR_P(getThis());
    rule_t *rule = &(pr->rule);

    zend_bool byday = 1;
    zval *dates, **data;
    HashTable *dates_hash;
    HashPosition pointer;
    int dates_count;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|b", &dates, &byday)) {
        return;
    }

    dates_hash = Z_ARRVAL_P(dates);
    dates_count = zend_hash_num_elements(dates_hash);

    datetime_t *eds, *pe;
    pe = eds = emalloc(sizeof(datetime_t)*dates_count);
    zend_hash_internal_pointer_reset_ex(dates_hash, &pointer);
    while (SUCCESS == zend_hash_get_current_data_ex(dates_hash, (void**)&data, &pointer)) {
        zval ztmp = **data;
        zval_copy_ctor(&ztmp);
        convert_to_double(&ztmp);
        //if (Z_TYPE(ztmp) == IS_DOUBLE)
        double t = Z_DVAL(ztmp);
        *pe++ = sdt_unix((__int64_t)t);

        zend_hash_move_forward_ex(dates_hash, &pointer);
    }

    rule->exdates.dates = eds;
    rule->exdates.size = dates_count;
    if (!tc_sorted(&(rule->exdates))) {
        efree(eds);
        rule->exdates.dates = NULL;
        rule->exdates.size = 0;
        RETURN_FALSE;
    }

    rule->exdates.byday = byday;
    RETURN_TRUE;
}

/* {{{ proto Recurr::dump(...) */
PHP_METHOD(Recurr, dump)
{
    php_recurr_obj *pr = Z_PHPRECURR_P(getThis());
    rule_t *rule = &(pr->rule);

    array_init(return_value);
    add_assoc_double(return_value, "start", dt_unix(rule->start));
    add_assoc_double(return_value, "end", dt_unix(rule->end));
    add_assoc_double(return_value, "until", dt_unix(rule->until));
    add_assoc_long(return_value, "freq", rule->freq);
    add_assoc_long(return_value, "interval", rule->interval);
    add_assoc_long(return_value, "monthfix", rule->monthfix);
    add_assoc_bool(return_value, "exbyday", rule->exdates.byday);

    zval exdates;
    ////ALLOC_INIT_ZVAL(exdates);
    array_init(&exdates);
    int i;
    for (i=0; i<rule->exdates.size; i++) {
        add_next_index_double(&exdates, dt_unix(rule->exdates.dates[i]));
    }
    add_assoc_zval(return_value,"exdates", &exdates);
}
/* }}} */


/* {{{ proto Recurr::constraint(...) */
PHP_METHOD(Recurr, constraint)
{
    php_recurr_obj *pr = Z_PHPRECURR_P(getThis());
    rule_t *rule = &(pr->rule);

    double dafter = rule->start;
    double dbefore = rule->end;
    int limit = 1;
    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ddl", &dafter, &dbefore, &limit)) {
        return;
    }

    array_init(return_value);
    __int64_t after = dafter;
    __int64_t before = dbefore;
    rule_constraint(return_value, rule, sdt_unix(after), sdt_unix(before), limit);
}
/* }}} */

void rule_constraint(zval *rv, rule_t *r, datetime_t after, datetime_t before, int limit) {
    const int duraSeconds[] = {1, 60, 60*60, 60*60*24, 60*60*24*7};
    if (after < r->start) {
        after = r->start;
    }
    if (before > r->until) {
        before = r->until;
    }

    if (r->exdates.size > 0) {
        tc_reset(&(r->exdates), after);
    }

    __int64_t duration = r->end - r->start;

    if (r->freq < MONTHLY) {
        __int64_t dura = duraSeconds[r->freq] * r->interval;

        datetime_t rc = after;
        if (after > r->start) {
            //after -= r->start;
            rc = rc + (after - r->start + dura - 1) / dura * dura;
        }

        while (rc <= before && limit != 0) {
            datetime_t rcend = rc + duration;
            if (rcend < before && !tc_isexclude(&(r->exdates), rc)) {
                add_next_index_double(rv, dt_unix(rc)); // set rc -> rcend
                limit--;
            }
            rc = rc + ((duration/dura + 1) * dura);
        }
    } else {
        int ay = 0, am = 0;
        datetime_t rc = r->start;//日程开始时间
        int year, month, day;
        dt_date(rc, &year, &month, &day);
        __int64_t dsecond = rc % 86400;

        if( r->lunarflag == Solar) {
            while (rc < before && limit != 0) {//在范围内
                if (r->freq == MONTHLY) {
                    am += r->interval;      //月加一
                } else if (r->freq == YEARLY) {
                    ay += r->interval;      //年加一
                }
    
                datetime_t rcend = rc + duration;//本次循环的日程结束时间戳
                if (rc >= after && rcend <= before) {//日程在范围内，（日程的整个范围在里面）
                    if (!tc_isexclude(&(r->exdates), rc)) {//是否期望去掉的
                        add_next_index_double(rv, dt_unix(rc)); //  set rc -> rcend
                        limit--;
                    }
                } else {
                    break;
                }
    
                
                datetime_t rt = sdt_datefix(year+ay, month+am, day, r->monthfix);//将日期粗暴变化进行修复
    
                rt += dsecond;
    
                rc = rt;
            }
        } else {//农历版
            triger_result_t triger_result;
            triger_result.leapstamp = 0;
            while (rc < before && limit != 0) {//在范围内
                if (r->freq == MONTHLY) {
                    am += r->interval;      //月加一
                } else if (r->freq == YEARLY) {
                    ay += r->interval;      //年加一
                }
    
                datetime_t rcend = rc + duration;//本次循环的日程结束时间戳
                if (rc >= after && rcend <= before) {//日程在范围内，（日程的整个范围在里面）
                    if (!tc_isexclude(&(r->exdates), rc)) {//是否期望去掉的
                        add_next_index_double(rv, dt_unix(rc)); //  set rc -> rcend
                        limit--;
                        //判断是否为年间隔，且leapstamp是否不为0，如有则增加闰月的触发
                        if (r->freq == YEARLY && triger_result.leapstamp != 0) {
                            rc = sdt_unix(triger_result.leapstamp);
                            if (rc >= after && rcend <= before) {//日程在范围内，（日程的整个范围在里面）
                                if (!tc_isexclude(&(r->exdates), rc)) {//是否期望去掉的
                                    add_next_index_double(rv, dt_unix(rc)); //  set rc -> rcend
                                    limit--;
                                }
                            }
                        }
                    }
                } else {
                    break;
                }
    
                
                if (r->freq == YEARLY ){
                    add_lunaryear(year, month, day, ay, &triger_result);
                    triger_result.stamp += dsecond;
                    if( triger_result.leapstamp != 0){
                        triger_result.leapstamp += dsecond;
                    }
                    rc = sdt_unix(triger_result.stamp);
                }else if(r->freq == MONTHLY) {
                    __int64_t tr = add_lunarmonth(year, month, day, am);
                    rc = sdt_unix(tr+dsecond);
                }
    
            }
        }
    }
}


