#!/usr/bin/env ruby

require 'json'

dst = JSON.parse(File.read("schema/mergeintome.json"))
src = JSON.parse(File.read("schema/mergeme.json"))
merger = proc { |key,v1,v2| Hash === v1 && Hash === v2 ? v1.merge(v2, &merger) : v2 }
dst.merge(src, &merger)
