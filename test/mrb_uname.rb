##
## Uname Test
##

assert("Uname#sysname") do
  assert_equal(false, Uname.sysname.nil?)
end
