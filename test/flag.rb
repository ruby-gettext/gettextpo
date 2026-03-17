assert 'basic' do
  assert_raise NoMethodError do
    GettextPO::FlagIterator.new
  end

  file = GettextPO::File.new
  message = file.message_iterator.insert('msgid1', 'msgstr1')
  message.respond_to?(:update_workflow_flag) or next true

  message.update_workflow_flag('fuzzy')
  iter = message.workflow_flag_iterator
  assert_equal 'fuzzy', iter.next
  assert_raise StopIteration do
    iter.next
  end

  assert_equal ['fuzzy'], message.workflow_flag_iterator.each.to_a

  message.update_sticky_flag('no-wrap')
  iter = message.sticky_flag_iterator
  assert_equal ['no-wrap'], iter.to_a

  true
end
