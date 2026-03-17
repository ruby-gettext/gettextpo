# Copyright (C) 2026  gemmaro
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

assert 'new' do
  assert_raise(NoMethodError) do
    GettextPO::Message.new
  end
  true
end

assert 'getters and setters' do
  path = File.expand_path(File.join(__FILE__, "../../test.cruby/resources/ok.po"))
  iterator = GettextPO::File.read(path).message_iterator
  message = iterator.next
  message.msgctxt = "msgctxt1"
  assert_equal "msgctxt1", message.msgctxt
  message.msgid = "msgid1"
  assert_equal "msgid1", message.msgid
  message.msgid_plural = "msgid_plural1"
  assert_equal "msgid_plural1", message.msgid_plural
  message.msgstr = "msgstr1"
  assert_equal "msgstr1", message.msgstr
  message.set_msgstr_plural(1, "msgstr_plural1")
  assert_equal "msgstr_plural1", message.msgstr_plural(1)
  message.comments = "comments1"
  assert_equal "comments1\n", message.comments
  message.extracted_comments = "extracted_comments1"
  assert_equal "extracted_comments1\n", message.extracted_comments
  message.prev_msgctxt = "prev_msgctxt1"
  assert_equal "prev_msgctxt1", message.prev_msgctxt
  message.prev_msgid = "prev_msgid1"
  assert_equal "prev_msgid1", message.prev_msgid
  message.prev_msgid_plural = "prev_msgid_plural1"
  assert_equal "prev_msgid_plural1", message.prev_msgid_plural
  message.obsolete = true
  assert_true message.obsolete?
  message.fuzzy = true
  assert_true message.fuzzy?

  if message.respond_to?(:format)
    assert_nil message.format('c-format')
  end
  message.update_format("c-format")
  if message.respond_to?(:format)
    assert_true message.format('c-format')
  end
  assert_true message.format?("c-format")
  message.update_format("objc-format", opposite: true)
  if message.respond_to?(:format)
    assert_false message.format('objc-format')
  end
  assert_false message.format?("objc-format")
  message.update_format("csharp-format")
  message.update_format("csharp-format", remove: true)
  assert_false message.format?("csharp-format")
  assert_raise(GettextPO::Error) do
    message.update_format("c++-format", opposite: true, remove: true)
  end
  message.range = 1..2
  assert_true message.range?(1..2)
  true
end

assert 'flags' do
  path = File.expand_path(File.join(__FILE__, "../../test.cruby/resources/ok.po"))
  iterator = GettextPO::File.read(path).message_iterator
  message = iterator.next
  message.respond_to?(:workflow_flag?) or next true
  assert_false message.workflow_flag?('fuzzy')
  message.update_workflow_flag('fuzzy')
  assert_true message.workflow_flag?('fuzzy')
  message.update_workflow_flag('fuzzy', set: false)
  assert_false message.workflow_flag?('fuzzy')
  message.update_sticky_flag('no-wrap')
  assert_true message.sticky_flag?('no-wrap')
  true
end

assert 'each flag' do
  path = File.expand_path(File.join(__FILE__, "../../test.cruby/resources/flag.po"))
  iterator = GettextPO::File.read(path).message_iterator
  message = iterator.next

  assert_equal ['no-wrap'], message.each_sticky_flag.to_a
  assert_equal ['fuzzy'], message.each_workflow_flag.to_a

  true
end
