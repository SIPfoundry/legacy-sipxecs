#!/bin/sh

mkdir backup-mailstore backup-configs
echo "mock config backup" > backup-configs/fs.tar.gz
echo "mock database backup" > backup-configs/pds.tar.gz
echo "mock mailstore backup" > backup-mailstore/mailstore.tar.gz
