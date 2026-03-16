#--
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
#++

# The main entrypoints to parse PO files are GettextPO::File.new and
# GettextPO::File.read.
#
# == Error handling
#
# There are two kinds of errors in this gem.  The first is for
# exception handlings of libgettextpo.  The second is regular error
# raising from this gem.
#
# Some functions takes exception error handling paramters like
# +xerror+ and +xerror2+.  These parameters expect +Proc+ object which
# takes several parameters.  See also the description of
# GettextPO::File.read method.  The +severity+ parameter among these
# parameters is either SEVERITY_WARNING, SEVERITY_ERROR, or
# SEVERITY_FATAL_ERROR.
#
# This gem normally raises +GettextPO::Error+ object, except for the
# standard ones, +StopIteration+ for example.
#
module GettextPO
  Error = Class.new(StandardError) # :nodoc:

  # This class doesn't provide the +new+ class method.  See also
  # GettextPO::File#message_iterator.
  class MessageIterator
    def self.new # :nodoc:
      raise NoMethodError,
            "please use other methods instead, such as GettextPO::File#message_iterator"
    end

    def each # yields: message
      while true
        begin
          yield self.next
        rescue StopIteration
          return self
        end
      end
    end

    include Enumerable
  end

  # This class doesn't provide the +new+ class method.  Refer to
  # GettextPO::MessageIterator#next or
  # GettextPO::MessageIterator#insert.
  class Message
    def self.new # :nodoc:
      raise NoMethodError,
            "please use other methods instead, such as GettextPO::MessageIterator#next"
    end

    def file_positions
      positions = []
      index = 0
      while (pos = filepos(index))
        positions << pos
        index += 1
      end
      positions
    end
  end

  class File
    def each_message(domain = nil) # yields: message
      iter = message_iterator(domain)
      while true
        begin
          yield iter.next
        rescue StopIteration
          return
        end
      end
    end
  end

  # This class doesn't provide the +new+ class method.  See also
  # GettextPO::Message#filepos.
  class FilePos
    def self.new # :nodoc:
      raise NoMethodError,
            "please use other methods instead, such as GettextPO::Message#filepos"
    end
  end

  # This class doesn't provide the +new+ class method.  See also
  # GettextPO::Message#workflow_flag_iterator and
  # GettextPO::Message#sticky_flag_iterator.
  class FlagIterator
    def self.new # :nodoc:
      raise NoMethodError,
            "please use other methods instead, such as GettextPO::Message#workflow_flag_iterator"
    end

    def each # yields: flag
      while true
        begin
          yield self.next
        rescue StopIteration
          return self
        end
      end
    end

    include Enumerable
  end
end
