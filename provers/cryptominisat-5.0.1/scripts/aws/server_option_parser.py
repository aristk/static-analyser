#!/usr/bin/env python
# -*- coding: utf-8 -*-

import optparse
import random
import time
import string


def parse_arguments():
    class PlainHelpFormatter(optparse.IndentedHelpFormatter):

        def format_description(self, description):
            if description:
                return description + "\n"
            else:
                return ""

    usage = """usage: %prog

For the --solver options you can give:
* SWDiA5BY.alt.vd.res.va2.15000.looseres.3tierC5/binary/SWDiA5BY_static
* lingeling_ayv/binary/lingeling"""
    parser = optparse.OptionParser(usage=usage, formatter=PlainHelpFormatter())
    parser.add_option("--verbose", "-v", action="store_true",
                      default=False, dest="verbose", help="Be more verbose"
                      )

    parser.add_option("--numclients", "-c", default=1, type=int,
                      dest="client_count", help="Number of clients to launch"
                      )

    parser.add_option("--port", "-p", default=10000, dest="port",
                      help="Port to listen on. [default: %default]", type="int"
                      )

    parser.add_option("--tout", "-t", default=1500, dest="timeout_in_secs",
                      help="Timeout for the file in seconds"
                      "[default: %default]",
                      type=int
                      )

    parser.add_option("--toutmult", default=12.1, dest="tout_mult",
                      help="Approx: 1x is solving, 10x time is DRAT time wait, 1x is parsing, 0.1x that is sending us the result."
                      "[default: %default]",
                      type=float
                      )

    parser.add_option("--memlimit", "-m", default=1600, dest="mem_limit_in_mb",
                      help="Memory limit in MB"
                      "[default: %default]",
                      type=int
                      )

    parser.add_option("--cnflist", default="satcomp14", dest="cnf_list",
                      type=str,
                      help="The list of CNF files to solve, first line the dir"
                      "[default: %default]",
                      )

    parser.add_option("--dir", default="/home/ubuntu/", dest="base_dir", type=str,
                      help="The home dir of cryptominisat"
                      )

    parser.add_option("--solver",
                      default="cryptominisat/build/cryptominisat5",
                      dest="solver",
                      help="Solver executable"
                      "[default: %default]",
                      type=str
                      )

    parser.add_option("--s3bucket", default="msoos-solve-results",
                      dest="s3_bucket", help="S3 Bucket to upload finished data"
                      "[default: %default]",
                      type=str
                      )

    parser.add_option("--s3folder", default="results", dest="s3_folder",
                      help="S3 folder name to upload data"
                      "[default: %default]",
                      type=str
                      )

    parser.add_option("--git", dest="git_rev", type=str,
                      help="The GIT revision to use. Default: HEAD"
                      )

    parser.add_option("--opt", dest="extra_opts", type=str, default="",
                      help="Extra options to give to solver"
                      "[default: %default]",
                      )

    parser.add_option("--noshutdown", "-n", default=False, dest="noshutdown",
                      action="store_true", help="Do not shut down clients"
                      )

    parser.add_option("--drat", default=False, dest="drat",
                      action="store_true", help="Use DRAT"
                      )

    parser.add_option("--stats", default=False, dest="stats",
                      action="store_true", help="Use STATS and get SQLITE data"
                      )

    parser.add_option("--gauss", default=False, dest="gauss",
                      action="store_true", help="Use GAUSS"
                      )

    parser.add_option("--logfile", dest="logfile_name", type=str,
                      default="python_server_log.txt", help="Name of LOG file")

    # parse options
    options, args = parser.parse_args()

    def rnd_id():
        return ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(5))

    options.logfile_name = options.base_dir + options.logfile_name
    options.s3_folder += "-" + time.strftime("%d-%B-%Y")
    options.s3_folder += "-%s" % rnd_id()

    return options, args

if __name__ == "__main__":
    options, args = parse_arguments()
    print("Options are:", options)
    print("args are:", args)
