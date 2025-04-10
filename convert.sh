#!/usr/bin/env bash

set -euo pipefail

MAPTOOL=./cmake-build-debug/croctool/croctool

mkdir -p resources/maps/{json,map,wad}/{beta012,pc,sles00593,slps01055,slus00530}

# Unwad everything first
for i in resources/maps/wad/slus00530/*.[Ww][Aa][Dd]; do
    ${MAPTOOL} map unwad $i resources/maps/wad/slus00530/$(echo "$i" | perl -pe 's/.+MAP(\d+)\.[wW][aA][dD]$/MP0\1_/g');
    if [ $? -ne 0 ]; then
        echo "> $i failed"
    fi
done

for i in resources/maps/map/beta012/*.[Mm][Aa][Pp]; do
	${MAPTOOL} map decompile "$i" resources/maps/json/beta012/"$(basename "$i" .MAP).json";
    if [ $? -ne 0 ]; then
        echo "> $i failed"
    fi
done

for i in resources/maps/map/sles00593/*.[Mm][Aa][Pp]; do
	${MAPTOOL} map decompile "$i" resources/maps/json/sles00593/"$(basename "$i" .MAP).json";
    if [ $? -ne 0 ]; then
        echo "> $i failed"
    fi
done

for i in resources/maps/map/slps01055/*.[Mm][Aa][Pp]; do
	${MAPTOOL} map decompile "$i" resources/maps/json/slps01055/"$(basename "$i" .MAP).json";
    if [ $? -ne 0 ]; then
        echo "> $i failed"
    fi
done

for i in resources/maps/map/slus00530/*.[Mm][Aa][Pp]; do
	${MAPTOOL} map decompile "$i" resources/maps/json/slus00530/"$(basename "$i" .MAP).json";
    if [ $? -ne 0 ]; then
        echo "> $i failed"
    fi
done

for i in resources/maps/map/pc/*.[Mm][Aa][Pp]; do
    ${MAPTOOL} map decompile "$i" resources/maps/json/pc/"$(basename "$i" .MAP).json";
	if [ $? -ne 0 ]; then
		echo "> $i failed"
	fi
done
