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

require_relative "lib/gettextpo/version"

Gem::Specification.new do |spec|
  spec.name = "gettextpo"
  spec.version = GettextPO::VERSION
  spec.authors = ["gemmaro"]
  spec.email = ["gemmaro.dev@gmail.com"]

  spec.summary = "GNU gettext PO file parser library"
  spec.description = "This gem is a Ruby library for the GNU gettext PO files.  This is a C binding of the libgettextpo library, which is part of the GNU gettext package."
  spec.homepage = "https://git.disroot.org/gemmaro/ruby-gettextpo"
  spec.license = "GPL-3.0-or-later" # https://spdx.org/licenses/
  spec.required_ruby_version = ">= 3.1.0"
  spec.requirements << 'libgettextpo'

  spec.metadata["homepage_uri"] = spec.homepage
  spec.metadata["source_code_uri"] = "https://git.disroot.org/gemmaro/ruby-gettextpo.git"

  spec.files = [
    'COPYING', 'Gemfile', 'Rakefile', 'mrblib/mrb_gettextpo_common.rb',
    *Dir['*.md'], *Dir['ext/**/*.rb'], *Dir['ext/**/*.c'],
    *Dir['ext/**/*.h'], *Dir['lib/**/*.rb'],
  ]
  spec.require_paths = ["lib"]
  spec.extensions = ["ext/gettextpo/extconf.rb"]
  spec.metadata = {
    'rubygems_mfa_required' => 'true',
    'bug_tracker_uri'       => "#{spec.homepage}/issues",
    'changelog_uri'         => "#{spec.homepage}/src/branch/main/CHANGELOG.md",
    'documentation_uri'     => "https://gemmaro.github.io/ruby-gettextpo/",
    'homepage_uri'          => spec.homepage,
    'source_code_uri'       => spec.homepage,
    'wiki_uri'              => "#{spec.homepage}/wiki",
  }
end
