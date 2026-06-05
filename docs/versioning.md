# Versioning

`wn_zephyr_modules` uses date-based release tags for shared-module integration
points:

```text
vYYYY-MM-DD
```

Applications should pin `west.yml` to a release tag or a reviewed commit SHA,
not to a moving branch. A release should include:

- a committed code change,
- release notes under `docs/releases/`,
- any relevant design or ADR updates,
- validation notes for affected downstream applications.

## Current Release

`v2026-06-05` is the first documented `rt_config` file-export performance
release. FUM applications may opt into compact export with
`CONFIG_RT_CONFIG_FILE_EXPORT_COMMENTS=n`.
