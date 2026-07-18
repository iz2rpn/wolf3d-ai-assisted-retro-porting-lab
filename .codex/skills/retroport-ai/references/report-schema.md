# Evidence report contract

`tools/retroport.py analyze` produces schema version 1. Read the JSON report as
the fact layer and the Markdown report as a presentation of the same evidence.

## Top-level fields

- `project`: configured name, case study, and target.
- `source_metrics`: deterministic counts for the configured public source
  roots. These are not whole-repository language statistics.
- `dependencies`: names, upstream URLs, and pinned commits parsed from the
  source-fetch script.
- `patches`: changed files, hunks, additions, deletions, and SHA-256 per patch.
- `entry_points`: regex-discovered candidate entry points with `path` and
  `line`. Validate important candidates in source before drawing a flow graph.
- `finding_summary`: counts of evidence matches, not counts of bugs.
- `findings`: one rule match with category, severity, location, excerpt, and
  guidance.
- `release_check`: required-file and private/generated-asset policy result.
- `interpretation_contract`: the intended separation between deterministic
  facts, GPT-5.6 synthesis, and human authority.

## Interpretation rules

1. Cite evidence as `relative/path:line`.
2. Treat severity as portability coupling, not security severity.
3. Deduplicate matches that describe one design decision.
4. Label a conclusion as an inference when it combines or extends facts.
5. Confirm license conclusions against actual license text; filename detection
   alone is insufficient.
6. Do not infer runtime frequency, memory, latency, or stability from source
   matches or binary size.
7. Preserve unavailable benchmark fields as `null` with a reason.
8. Never place private asset names, hashes, or host paths in a public report
   unless needed for a lawful, explicitly approved disclosure.

## Minimum GPT-5.6 output

The model output must include observed architecture, a cited evidence table,
inferences, unknowns, a ranked migration plan, one bounded experiment, a human
review gate, and exact verification commands. A generic summary without cited
evidence does not satisfy the workflow.

