#!/bin/sh

echo "Setting up weather data files..."

mkdir ./txt
cd ./txt || exit
touch andnowtheweather.txt forecast.txt temperature.txt chance.txt maxmin.txt
