/**
 * By @Qinwusui
 */
package com.wusui.plugin

import io.ktor.client.*
import io.ktor.client.call.*
import io.ktor.client.engine.cio.*
import io.ktor.client.plugins.compression.*
import io.ktor.client.plugins.contentnegotiation.*
import io.ktor.client.request.*
import io.ktor.http.*
import io.ktor.serialization.kotlinx.json.*
import io.ktor.server.application.*
import io.ktor.server.request.*
import io.ktor.server.response.*
import io.ktor.server.routing.*
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.distinctUntilChanged
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.flow.flowOn
import kotlin.coroutines.CoroutineContext

val weatherClient = HttpClient(CIO) {
    install(ContentNegotiation) {
        json()
    }
    install(ContentEncoding) {
        gzip(1f)
    }
}

fun Routing.weather() {
    val key = ""

    route("/iot") {
        get("/weather") {
            println("发来请求")
            println(call.request.uri)
            val location = call.parameters["location"] ?: return@get
            val k = call.parameters["key"] ?: key
            getLocation(location, k).collect { stringFlow ->
                stringFlow.collect {
                    call.respond(it)
                }
            }
        }
    }
}

suspend fun <T> flowBYIO(scope: CoroutineContext = Dispatchers.IO, block: suspend () -> T) = flow {
    try {
        emit(block())
    } catch (_: Exception) {
    }
}.flowOn(scope).distinctUntilChanged()

suspend fun getLocation(loc: String, k: String) = flowBYIO {
    val request = weatherClient.request("https://geoapi.qweather.com/v2/city/lookup") {
        method = HttpMethod.Get
        parameter("key", k)
        parameter("location", loc)
    }
    val locId = request.body<LocationData>().location[0].id
    getWeather(locId, k)
}

suspend fun getWeather(locId: String, k: String) = flowBYIO {
    val request = weatherClient.request("https://devapi.qweather.com/v7/weather/now") {
        method = HttpMethod.Get
        parameter("key", k)
        parameter("location", locId)
        parameter("lan","en")
    }
    request.body<LocationWeather>()
}