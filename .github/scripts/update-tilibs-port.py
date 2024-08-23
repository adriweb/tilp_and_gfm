#!/usr/bin/env python3
"""Freeze the pushed experimental2 tip into the port before vcpkg cache lookup."""

import hashlib
import json
import os
from pathlib import Path
import re
import time
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen


def download(url, headers):
    for attempt in range(3):
        try:
            with urlopen(Request(url, headers=headers), timeout=30) as response:
                return response.read()
        except (HTTPError, URLError, TimeoutError) as error:
            if isinstance(error, HTTPError) and error.code not in (429, 500, 502, 503, 504):
                raise
            if attempt == 2:
                raise
            time.sleep(attempt + 1)


def main():
    headers = {"User-Agent": "tilp-and-gfm-ci", "Accept": "application/vnd.github+json"}
    token = os.environ.get("GH_TOKEN") or os.environ.get("GITHUB_TOKEN")
    if token:
        headers["Authorization"] = "Bearer " + token
    metadata = json.loads(download(
        "https://api.github.com/repos/adriweb/tilibs/commits/experimental2", headers
    ))
    revision = metadata["sha"]
    if not re.fullmatch(r"[0-9a-f]{40}", revision):
        raise ValueError("Invalid tilibs commit SHA")

    # The archive request is public and does not receive the API credential.
    archive = download(
        f"https://codeload.github.com/adriweb/tilibs/tar.gz/{revision}",
        {"User-Agent": "tilp-and-gfm-ci"},
    )
    checksum = hashlib.sha512(archive).hexdigest()
    port = Path(__file__).resolve().parents[2] / ".cmake/vcpkg_overlay_ports/tilibs/portfile.cmake"
    source = port.read_text()
    source, refs = re.subn(r"(?m)^    REF [0-9a-f]{40}$", f"    REF {revision}", source)
    source, hashes = re.subn(r"(?m)^    SHA512 [0-9a-f]{128}$", f"    SHA512 {checksum}", source)
    if refs != 1 or hashes != 1:
        raise ValueError("Expected exactly one pinned tilibs revision and checksum")
    port.write_text(source)
    print(f"Using adriweb/tilibs experimental2 at {revision} (SHA512 {checksum})")


if __name__ == "__main__":
    main()
