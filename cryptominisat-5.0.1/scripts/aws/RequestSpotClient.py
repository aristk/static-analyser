#!/usr/bin/env python
# -*- coding: utf-8 -*-

import boto
import sys
import ConfigParser
import os
import socket
import pprint
import boto.ec2
from boto.ec2.connection import EC2Connection
from common_aws import *
import logging


class RequestSpotClient:
    def __init__(self, test, noshutdown=False, count=1):
        self.conf = ConfigParser.ConfigParser()
        self.count = count
        if test:
            self.conf.read('ec2-spot-instance-test.cfg')
            self.limit_create = 1
        else:
            self.conf.read('ec2-spot-instance.cfg')
            self.limit_create = 8

        self.ec2conn = self.__create_ec2conn()
        if self.ec2conn is None:
            print 'Unable to create EC2 ec2conn'
            sys.exit(0)

        self.user_data = self.__get_user_data(noshutdown)
        self.our_ids = []

    def __get_user_data(self, noshutdown):
        extra_args = ""
        if noshutdown:
            extra_args = " --noshutdown"
        user_data = """#!/bin/bash
set -e

apt-get update
apt-get -y install git python-boto
apt-get -y install cmake make g++ libboost-all-dev
apt-get -y install libsqlite3-dev awscli unzip
apt-get -y install linux-cloud-tools-generic linux-tools-generic
apt-get -y install linux-cloud-tools-3.13.0-53-generic linux-tools-3.13.0-53-generic

# Get CMS
cd /home/ubuntu/
sudo -H -u ubuntu bash -c 'ssh-keyscan github.com >> ~/.ssh/known_hosts'
sudo -H -u ubuntu bash -c 'git clone --no-single-branch --depth 50 https://github.com/msoos/cryptominisat.git'
# sudo -H -u ubuntu bash -c 'aws s3 cp s3://msoos-solve-data/solvers/features_to_reconf.cpp /home/ubuntu/cryptominisat/src/ --region=us-west-2'

# Get credentials
cd /home/ubuntu/
sudo -H -u ubuntu bash -c 'aws s3 cp s3://msoos-solve-data/solvers/.boto . --region=us-west-2'
sudo -H -u ubuntu bash -c 'aws s3 cp s3://msoos-solve-data/solvers/email.conf . --region=us-west-2'

# build solvers
sudo -H -u ubuntu bash -c '/home/ubuntu/cryptominisat/scripts/aws/build_swdia5by.sh >> /home/ubuntu/build.log'
sudo -H -u ubuntu bash -c '/home/ubuntu/cryptominisat/scripts/aws/build_swdia5by_old.sh >> /home/ubuntu/build.log'
sudo -H -u ubuntu bash -c '/home/ubuntu/cryptominisat/scripts/aws/build_lingeling_ayv.sh >> /home/ubuntu/build.log'
sudo -H -u ubuntu bash -c '/home/ubuntu/cryptominisat/scripts/aws/build_drat-trim2.sh >> /home/ubuntu/build.log'
sudo -H -u ubuntu bash -c '/home/ubuntu/cryptominisat/scripts/aws/build_glucose2016.sh >> /home/ubuntu/build.log'

# Start client
cd /home/ubuntu/cryptominisat
sudo -H -u ubuntu bash -c 'nohup /home/ubuntu/cryptominisat/scripts/aws/client.py %s > /home/ubuntu/log.txt  2>&1' &

DATA="%s"
""" % (extra_args, get_ip_address("eth0"))

        return user_data

    def __create_ec2conn(self):
        ec2conn = EC2Connection()
        regions = ec2conn.get_all_regions()
        for r in regions:
            if r.name == self.conf.get('ec2', 'region'):
                ec2conn = EC2Connection(region = r)
                return ec2conn
        return None

    def __provision_instances(self):
        reqs = self.ec2conn.request_spot_instances(price = self.conf.get('ec2', 'max_bid'),
                count = self.count,
                image_id = self.conf.get('ec2', 'ami_id'),
                subnet_id = self.conf.get('ec2', 'subnet_id'),
                instance_type = self.conf.get('ec2', 'type'),
                instance_profile_arn = self.conf.get('ec2', 'instance_profile_arn'),
                user_data = self.user_data,
                key_name = self.conf.get('ec2', 'key_name'),
                security_group_ids = [self.conf.get('ec2', 'security_group')])

        logging.info("Request created, got back IDs %s" % [r.id for r in reqs])
        return reqs

    def create_spots_if_needed(self):
        # Valid values: open | active | closed | cancelled | failed
        run_wait_spots = self.ec2conn.get_all_spot_instance_requests(filters={'state': 'open'})
        run_wait_spots.extend(self.ec2conn.get_all_spot_instance_requests(filters={'state': 'active'}))

        for spot in run_wait_spots:
            if spot.id in self.our_ids:
                logging.info("ID %s is either waiting or running, not requesting a new one" % spot.id)
                return

        if len(self.our_ids) >= self.limit_create:
            logging.error("Something really wrong has happened, we have reqested 4 spots aready! Not requesting more.")
            return

        self.create_spots()

    def create_spots(self):
        reqs = self.__provision_instances()

        for req in reqs:
            logging.info('New req state: %s ID: %s' % (req.state, req.id))
            self.our_ids.append(req.id)

        return self.our_ids
