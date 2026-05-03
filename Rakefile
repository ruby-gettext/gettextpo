# frozen_string_literal: true

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

require "bundler/gem_tasks"
require "rake/testtask"

Rake::TestTask.new(:test) do |t|
  t.libs << "test.cruby"
  t.libs << "lib"
  t.test_files = FileList["test.cruby/**/*_test.rb"]
end
task test: :compile

require "rake/extensiontask"

task build: :compile

GEMSPEC = Gem::Specification.load("gettextpo.gemspec")

Rake::ExtensionTask.new("gettextpo", GEMSPEC) do |ext|
  ext.lib_dir = "lib/gettextpo"
end

task default: %i[clobber compile test]
