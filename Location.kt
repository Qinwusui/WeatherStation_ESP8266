/** By @Qinwusui */

package com.wusui.plugin


import kotlinx.serialization.Serializable
@Serializable
data class Refer(
    val sources: List<String>,
    val license: List<String>
)
@Serializable
data class LocationData(
    val code: String, // 200
    val location: List<Location>,
    val refer: Refer
)
@Serializable
data class Location(
    val name: String, // 北京
    val id: String, // 101010100
    val lat: String, // 39.90498
    val lon: String, // 116.40528
    val adm2: String, // 北京
    val adm1: String, // 北京市
    val country: String, // 中国
    val tz: String, // Asia/Shanghai
    val utcOffset: String, // +08:00
    val isDst: String, // 0
    val type: String, // city
    val rank: String, // 10
    val fxLink: String // http://hfx.link/2ax1
)

@Serializable
data class LocationWeather(
    val code: String, // 200
    val updateTime: String, // 2020-06-30T22:00+08:00
    val fxLink: String, // http://hfx.link/2ax1
    val now: Now,
    val refer: Refer
)

@Serializable
data class Now(
    val obsTime: String, // 2020-06-30T21:40+08:00
    val temp: String, // 24
    val feelsLike: String, // 26
    val icon: String, // 101
    val text: String, // 多云
    val wind360: String, // 123
    val windDir: String, // 东南风
    val windScale: String, // 1
    val windSpeed: String, // 3
    val humidity: String, // 72
    val precip: String, // 0.0
    val pressure: String, // 1003
    val vis: String, // 16
    val cloud: String, // 10
    val dew: String // 21
)
