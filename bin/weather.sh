#!/bin/zsh

# if curl -s https://api.weather.gov/
#	then echo "Error connecting to the National Weather Service API." &&
#		echo "UNABLE TO CONNECT" >> ../andnowtheweather.txt
# fi

get_data () {
	curl -s -H "User-Agent: shell script for data retrieval" https://api.weather.gov/gridpoints/RAH/64,66/forecast |
		jq '.properties | .periods | .[0,2,4,6] | .name, .temperature, .shortForecast, .probabilityOfPrecipitation.value' > ~/weather/andnowtheweather.txt
}

get_data
