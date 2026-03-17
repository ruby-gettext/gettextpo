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
    GettextPO::MessageIterator.new
  end
  true
end

assert 'next' do
  path = File.expand_path(File.join(__FILE__, "../../test.cruby/resources/ok.po"))
  iterator = GettextPO::File.read(path).message_iterator
  assert_not_nil iterator.next
  assert_not_nil iterator.next
  assert_raise(StopIteration) do
    iterator.next
  end
  true
end

assert 'each' do
  path = File.expand_path(File.join(__FILE__, "../../test.cruby/resources/ok.po"))
  file = GettextPO::File.read(path)
  iterator = file.message_iterator
  messages = []
  iterator.each { |message| messages << message }
  assert_equal 2, messages.size

  iterator = file.message_iterator
  assert_equal ["", "msgid1"], iterator.each.map { |message| message.msgid }

  true
end

assert 'insert' do
  file = GettextPO::File.new
  iterator = file.message_iterator
  iterator.insert("msgid1", "msgstr1")
  path = File.expand_path(File.join(__FILE__, "../../test.cruby/tmp/insert.po"))
  file.write(path)
  assert_equal [["msgid1", "msgstr1"]],
               GettextPO::File.read(path).message_iterator
                 .map { |message| [message.msgid, message.msgstr] }
  true
end
