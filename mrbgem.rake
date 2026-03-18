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

MRuby::Gem::Specification.new('mruby-gettextpo') do |spec|
  spec.license = 'GPL-3.0-or-later'
  spec.authors = 'gemmaro'
  spec.summary = 'GNU gettext PO parser for mruby'
  spec.version = '0.1.0'
  spec.homepage = 'https://github.com/ruby-gettext/gettextpo'
  spec.linker.libraries << 'gettextpo'
  spec.add_test_dependency 'mruby-io'
end
