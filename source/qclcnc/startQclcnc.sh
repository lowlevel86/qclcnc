#!/bin/bash

currDir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
startingDir=$currDir

savedDir=$(head -n 1 .dir)
if [ -d "$savedDir" ]; then
	startingDir="$savedDir"
fi


choosenDir=$(dialog  --stdout --title "Choose G-code Directory" --fselect $startingDir 0 0)


sudo ./qclcnc "$choosenDir"


echo $choosenDir > "$currDir/.dir"
