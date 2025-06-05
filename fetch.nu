#!/usr/bin/env nu

let user = "tsoding"
let repo = "nob.h"
let branch = "main"
let header = "nob.h"

let url = "https://raw.githubusercontent.com/"
let url = $url | path join $user
let url = $url | path join $repo
let url = $url | path join $branch
let url = $url | path join $header

http get $url | save -f ./nob.h
