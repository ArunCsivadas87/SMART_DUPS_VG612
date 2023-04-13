# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES esp_http_client spiffs app_update log tcp_transport json esp_adc_cal)
set(COMPONENT_PRIV_REQUIRES lwip app_update freertos esp_event esp_wifi log nvs_flash tcp_transport nghttp esp_adc_cal)

set(COMPONENT_SRCS
	"ini.c"
	"Kpiprocess.c"
	)


set(COMPONENT_ADD_INCLUDEDIRS "." "include")


register_component()
