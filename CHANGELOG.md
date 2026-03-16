# Change log of Ruby GettextPO gem

## Unreleased

- Add `GettextPO::File#each_message` method.

## CRuby version 0.2.1 - 2026-03-16

- CRuby: Fix to work with gettext 0.23.

## CRuby and mruby version 0.2.0 - 2026-03-15

- CRuby and mruby: Add `GettextPO::Message#file_positions`,
  `GettextPO::Message#workflow_flag?`,
  `GettextPO::Message#update_workflow_flag`,
  `GettextPO::Message#workflow_flag_iterator`,
  `GettextPO::Message#sticky_flag?`,
  `GettextPO::Message#update_sticky_flag`, and
  `GettextPO::Message#sticky_flag_iterator` method.
- CRuby and mruby: Add `GettextPO::FlagIterator#next` method (and the
  class, which is `Enumerable`).
- mruby: Requires GNU gettext 1.0 or later.

## CRuby version 0.1.3

- CRuby: Fix file not found case.

## CRuby version 0.1.2 and mruby version 0.1.0 - 2026-03-11

- Add mruby version.
- CRuby: Fix exception error 2 handling.

## 0.1.1 - 2026-03-09

- Fix dangling pointer issue.
- Add libgettextpo version constant.
- Add severities constants.
- Improve API document.

## 0.1.0 - 2026-03-07

- Initial release
