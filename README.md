# RECURR 重复日程的 PHP 扩展实现

## 简介

RECURR 是一个用于快速计算重复日程的 PHP 扩展。

## 安装

有许多方式来构建此包，下面是两种最常用的方式：

-----------------------------------------------------------------------------
### 方式 1：跟 PHP 一起构建包

1.  在 PHP 源码文件夹下创建 ext/recurr 文件夹。将包中的所有文件复制到创建的文件夹下。

2.  运行

        ./buildconf

    来重新构建 PHP 的配置脚本。

3.  使用此选项来编译 PHP：

    `--enable-recurr' 以捆绑方式来构建 PHP 模块。

    `--enable-recurr=shared` 构建动态加载模块。

-----------------------------------------------------------------------------
### 方式 2：使用 phpize 工具来构建包

1.  解压此包。

2.  运行脚本

        phpize

    它将为构建 RECURR 扩展准备环境。

3.  运行

        ./configure --enable-recurr=shared

    以生成 makefile

4.  运行

        make

    以构建 RECURR 扩展库. 它将被置于 ./modules 目录下。

5.  运行

        make install

    以安装 RECURR 扩展库到 PHP 环境下。

-----------------------------------------------------------------------------
### 方式 3：通过 pecl 安装 (暂不支持)

1. 运行：

        pecl install recurr

    这就好了。

-----------------------------------------------------------------------------

## 使用

```php
<?php
function timeline($uinx) {
    print("{$uinx["year"]}.{$uinx["month"]}.{$uinx["day"]}_{$uinx["hour"]}.{$uinx["minute"]}.{$uinx["second"]}\n");
}

$now = time();
$r = new Recurr($now, $now+100, FREQ_DAILY, 1, $now+86400*100, 100);
$r->setExDates(array($now), false);
var_dump($r->dump());
$ret = $r->constraint($now, $now + 86400 * 50, 10);
foreach($ret as $v) {
    $dt = Recurr::datetime($v);
    timeline($dt);
}
```

## 接口文档

### 常量表

* FREQ_MINUTELY 频率 分
* FREQ_HOURLY 频率 小时
* FREQ_DAILY 频率 天
* FREQ_WEEKLY 频率 周
* FREQ_MONTHLY 频率 月
* FREQ_YEARLY 频率 年
* MFIX_IGNORE 月末不修复
* MFIX_LASTDAY 月末以最后一天修复

### RecurrInfo() -> array()

返回一个array()，包括扩展的版本信息

### Recurr::isleap(year int) -> bool

判断是否为润年

### Recurr::daysin(year int, month int) -> int

返回该年中该月的总天数

### Recurr::datetime(stamp double) -> array()

返回对应时间戳对应的年月日时分秒(year, month, day, hour, minute, second)

### Recurr构造函数

提交参数 | 类型 | 描述 |
------|
start | double | 日程开始时间戳 |
end | double | 日程结束时间戳 |
freq | enum | 日程重复视频(默认为WEEKLY) |
interval | int | 日程重复间隔(默认为) |
until | double | 日程结束时间戳(默认为) |
monthfix | enum | 月底修复方式(默认为) |

### Recurr->setExDates([]double stamps, byday bool) -> bool

设置例外时间

提交参数 | 类型 | 描述 |
------|
stamps | []double | 例外时间戳(必须从小到大排序) |
byday | bool | 以天进行匹配(默认为true) |

### Recurr->dump() -> array()

以array的形式返回Recurr的信息。

数组key | 类型 | 描述 |
------|
start | double | 第一次开始时间戳 |
end | double | 第一次结束时间戳 |
until | double | 结束时间戳 |
freq | int | 频率 |
interval | int | 间隔 |
monthfix | int | 月末修复方式 |
exbyday | bool | 例外时间处理方式 |
exdates | []double | 例外时间 |

### Recurr->constraint(after double, before double, limit int) -> array()

计算区间时间内的重复日程，返回所有重复日程的开始时间戳

| 提交参数 | 类型 | 描述 |
|------|
| after | double | 时间戳区间左 |
| before | double | 时间戳区间右 |
| limit | double | 限制条数(默认为1) |
