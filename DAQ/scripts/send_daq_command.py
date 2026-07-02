#!/usr/bin/env python3
"""Send a command to a CupDAQManager message server (ZMQ ROUTER, JSON protocol).

Mirrors the REQ client pattern used internally in CupDAQManager_rc.cc
(RC_TCB's execute_run lambda) so it talks the same wire protocol.

Requires pyzmq. System python3 on this host doesn't have it; use the
Anaconda one instead, e.g.:
    /home/cupsoft/Products/ANACONDA/V2023.07/bin/python3 send_daq_command.py -c kCONFIGRUN
"""

import argparse
import json
import sys

import zmq

COMMANDS = [
    "kQUERYDAQSTATUS", "kQUERYRUNINFO", "kQUERYTRGINFO", "kQUERYMONITOR",
    "kCONFIGRUN", "kSTARTRUN", "kENDRUN", "kSPLITOUTPUTFILE", "kSETERROR", "kEXIT",
]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("-H", "--host", default="127.0.0.1")
    parser.add_argument("-p", "--port", type=int, default=7100)
    parser.add_argument("-c", "--command", default="kCONFIGRUN", choices=COMMANDS)
    parser.add_argument("-t", "--timeout", type=int, default=3000, help="ms")
    args = parser.parse_args()

    ctx = zmq.Context()
    sock = ctx.socket(zmq.REQ)
    sock.setsockopt(zmq.RCVTIMEO, args.timeout)
    sock.setsockopt(zmq.LINGER, 0)

    endpoint = f"tcp://{args.host}:{args.port}"
    sock.connect(endpoint)

    req = {"command": args.command}
    sock.send_string(json.dumps(req))

    try:
        reply = sock.recv_string()
    except zmq.error.Again:
        print(f"no reply from {endpoint} within {args.timeout} ms", file=sys.stderr)
        sys.exit(1)

    print(json.dumps(json.loads(reply), indent=2))


if __name__ == "__main__":
    main()
