#! /usr/bin/env bash

declare -A hostnames

# hostnames[2]=1
# hostnames[10]=4
# hostnames[12]=4
# hostnames[29]=2
# hostnames[33]=4
# hostnames[34]=4
# hostnames[35]=3
# hostnames[37]=4
# hostnames[38]=4
# hostnames[41]=4
# hostnames[42]=4
# hostnames[43]=4
# hostnames[44]=4
# hostnames[45]=4
# hostnames[46]=4
# hostnames[47]=4
# hostnames[48]=4
# hostnames[83]=10
# hostnames[84]=10
# hostnames[85]=10
# hostnames[86]=10
# hostnames[87]=10
# hostnames[88]=10
# hostnames[89]=10
# hostnames[90]=10
# hostnames[91]=10
# hostnames[92]=10
# hostnames[93]=10
# hostnames[94]=10
# hostnames[95]=10
# hostnames[96]=10


for hostname in "${!hostnames[@]}"
do
	ssh ogila@circinus-${hostname}.ics.uci.edu << EOF
		cd Research/ZipTree/Experiments && bash runexperiments.sh 1 "${hostnames[$hostname]}"
EOF
done
