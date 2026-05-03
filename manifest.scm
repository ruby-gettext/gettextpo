;; Copyright (C) 2026  gemmaro
;;
;; This program is free software: you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <https://www.gnu.org/licenses/>.

(use-modules (guix packages)
             (guix download)
             (guix profiles)
             (gnu packages gettext)
             (gnu packages gdb)
             (gnu packages commencement)
             (gnu packages llvm)
             (gnu packages ruby)
             (gnu packages ruby-check)
             (gnu packages ruby-xyz)
             (gnu packages base))

(define-public gnu-gettext-1.0
  (package/inherit gnu-gettext
    (version "1.0")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://gnu/gettext/gettext-" version
                                  ".tar.gz"))
              (sha256 (base32
                       "06jgjxnm5xhw0d0zwl9birlwfxfgfqhk9q029j3h9941r5wrpnc5"))))))

(packages->manifest (list gnu-gettext-1.0
                          gcc-toolchain
                          gdb
                          clang
                          ruby-3.3
                          ruby-rake-compiler
                          ruby-ruby-memcheck
                          gnu-make))
