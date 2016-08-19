assert("Kernel.at_exit") do
  assert_raise(ArgumentError) { at_exit }
  assert_raise(ArgumentError) { Kernel.at_exit }
  at_exit {}
  Kernel.at_exit {}
end
