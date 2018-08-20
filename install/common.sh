#!/usr/bin/env bash
if [ -z "$NAO_HOME" ] ; then
  echo "NAO_HOME not set, exiting"
  exit 1
fi
set -e
source $NAO_HOME/install/vars
cd $NAO_HOME/install
