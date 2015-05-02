#!/bin/sh
tr -dc A-Za-z0-9 < /dev/urandom|dd bs=31 count=1 2>/dev/null
echo
