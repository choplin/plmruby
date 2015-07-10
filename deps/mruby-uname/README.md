# mruby-uname   [![Build Status](https://travis-ci.org/matsumoto-r/mruby-uname.svg?branch=master)](https://travis-ci.org/matsumoto-r/mruby-uname)
Uname class, system uname bindings
## install by mrbgems
- add conf.gem line to `build_config.rb`

```ruby
MRuby::Build.new do |conf|

    # ... (snip) ...

    conf.gem :git => 'https://github.com/matsumoto-r/mruby-uname.git'
end
```
## example
```ruby
$ ./bin/mirb
mirb - Embeddable Interactive Ruby Shell

> Uname.version
 => "#46-Ubuntu SMP Thu Apr 10 19:11:08 UTC 2014"
> Uname.release
 => "3.13.0-24-generic"
> Uname.sysname
 => "Linux"
> Uname.nodename
 => "ubuntu14-04-64"
> Uname.machine
 => "x86_64"
>
```

## License
under the MIT License:
- see LICENSE file
