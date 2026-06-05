# ADR 0001: rt_config File Export Format Is Application Selectable

Date: 2026-06-05

Status: Accepted

## Context

The original `rt_config` file exporter wrote a human-readable comment block for
each setting and performed several filesystem writes per setting. That was
acceptable for small setting files, but FUM PDCM exposed long save latency with
a large setting set stored on FAT over external SPI NOR flash.

CricketNode also uses `rt_config`, so changing the default export format would
risk breaking an existing application expectation.

## Decision

Keep commented export as the default and add a Kconfig switch for compact
`name=value` export:

```text
CONFIG_RT_CONFIG_FILE_EXPORT_COMMENTS
```

Also coalesce each exported item into one bounded buffer before writing it to
the filesystem and check write/close errors.

## Consequences

- Existing applications keep the same file shape by default.
- FUM can opt into compact export to reduce save latency and flash traffic.
- Import stays compatible with both formats because comments and blank lines are
  ignored.
- Future filesystem tests should cover commented export, compact export, partial
  write failure, and close failure paths.
