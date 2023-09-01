#!/bin/zsh

# if curl -s https://api.weather.gov/
#	then echo "Error connecting to the National Weather Service API." &&
#		echo "UNABLE TO CONNECT" >> ../andnowtheweather.txt
# fi

get_data () {
	curl -s -H -f --connect-timeout 30 --user-agent "zsh script on macOS (contact: galando@protonmail.com)" https://api.weather.gov/gridpoints/RAH/64,66/forecast |
		jq '.properties | .periods | .[0,2,4] | .name, .temperature, .shortForecast, .probabilityOfPrecipitation.value' > "../txt/andnowtheweather.txt"

	curl -s -f 'https://api.open-meteo.com/v1/forecast?latitude=35.994&longitude=-78.8986&hourly=temperature_2m&daily=temperature_2m_max,temperature_2m_min&timezone=GMT' |
		jq '.daily | .temperature_2m_max | .[0,1,2]' > "../txt/maxmin.txt"

	curl -s -f 'https://api.open-meteo.com/v1/forecast?latitude=35.994&longitude=-78.8986&hourly=temperature_2m&daily=temperature_2m_max,temperature_2m_min&timezone=GMT' |
			jq '.daily | .temperature_2m_min | .[0,1,2]' >> "../txt/maxmin.txt"
}

parse_temp () {
	awk 'NR==2 || ((NR/2) % 2 == 1)' "../txt/andnowtheweather.txt" > "../txt/temperature.txt"
}

parse_forecast () {
	awk 'NR==3 || ((NR+1) % 4 == 0)' "../txt/andnowtheweather.txt" > "../txt/forecast.txt"
}

parse_chance_rain () {
	awk 'NR==4 || (NR % 4 == 0)' "../txt/andnowtheweather.txt" > "../txt/chance.txt"
}

get_data
parse_temp
parse_forecast
parse_chance_rain
