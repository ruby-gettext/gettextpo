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
  assert_kind_of GettextPO::File, GettextPO::File.new
  true
end

assert "read" do
  path = File.expand_path(File.join(__FILE__, "../../test.cruby/resources/ok.po"))
  assert_kind_of GettextPO::File, GettextPO::File.read(path)

  assert_raise(GettextPO::Error) do
    GettextPO::File.read(File.join(__FILE__, '../not-there'))
  end

  xerrors = []
  assert_raise(GettextPO::Error) do
    path = File.expand_path(File.join(__FILE__, "../../test.cruby/resources/a.po"))
    GettextPO::File.read(path,
                         xerror: Proc.new { |**kwargs| xerrors << kwargs })
  end
  assert_equal 2, xerrors.size
  assert_equal "keyword \"a\" unknown", xerrors[0][:message_text]

  xerrors = []
  xerrors2 = []
  assert_raise(GettextPO::Error) do
    path = File.expand_path(File.join(__FILE__, "../../test.cruby/resources/bad.po"))
    GettextPO::File.read(path,
                         xerror: Proc.new { |**kwargs| xerrors << kwargs },
                         xerror2: Proc.new { |**kwargs| xerrors2 << kwargs })
  end
  assert_equal 0, xerrors.size
  assert_equal "duplicate message definition", xerrors2[0][:message_text1]
  true
end

assert "write" do
  path = File.expand_path(File.join(__FILE__, "../../test.cruby/tmp/write.po"))
  assert_nil GettextPO::File.new.write(path)
  true
end

assert 'domains' do
  path = File.expand_path(File.join(__FILE__, "../../test.cruby/resources/ok.po"))
  assert_equal ["messages"], GettextPO::File.read(path).domains
  true
end

assert 'message iterator' do
  assert_kind_of GettextPO::MessageIterator,
                 GettextPO::File.new.message_iterator
  true
end

assert 'each message' do
  path = File.expand_path(File.join(__FILE__, "../../test.cruby/resources/ok.po"))
  messages = []
  GettextPO::File.read(path).each_message { |message| messages << message.msgid }
  assert_equal ['', 'msgid1'], messages
  true
end

assert 'header field' do
  assert_nil GettextPO::File.new.domain_header

  header = <<~END_HEADER
    Project-Id-Version: Ruby GettextPO
    PO-Revision-Date: 2026-03-07 13:29+0900
    Last-Translator: gemmaro <gemmaro.dev@gmail.com>
    Language-Team: none
    MIME-Version: 1.0
    Content-Type: text/plain; charset=UTF-8
    Content-Transfer-Encoding: 8bit
  END_HEADER
  path = File.expand_path(File.join(__FILE__, "../../test.cruby/resources/ok.po"))
  assert_equal header, GettextPO::File.read(path).domain_header
  assert_nil GettextPO::File.read(path).domain_header("invalid domain")
  true
end
