#!/bin/bash

AAP2_SECRET="my_extremely_secret_secret_omg_i_love_this_secretly_secret_secret" ./ud3tn/result/bin/ud3tn \
  -b 7 \
  -c "sqlite:file::memory:?cache=shared;tcpclv3:*,4556;smtcp:*,4222,false;mtcp:*,4224" \
  -e dtn://peer-a.dtn/ \
  -l 86400 \
  -L 2 \
  -m 0 \
  -s $PWD/ud3tn-peera.socket \
  -S $PWD/ud3tn-peera.aap2.socket \
  -x "AAP2_SECRET"
