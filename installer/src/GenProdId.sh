#!/bin/bash
echo -n $1 | sha256sum | cut -c 1-32 | sed 's/./&-/8;s/./&-/13;s/./&-/18;s/./&-/23'

