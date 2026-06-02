#!/usr/bin/env bash
# Typo-tolerant aliases for `clear`.
# The most common accidental keystroke sequences are covered here.
# Generated via a compact loop instead of 100+ individual alias lines.

_clear_typos=(
    clera cler cleer cleear cleaer claer cleare clrea
    lcear lcar lcaer
    celar cear cealr
    clar clra clae
    cler1 clera1
)

for _t in "${_clear_typos[@]}"; do
    # shellcheck disable=SC2139
    alias "$_t"='clear'
done

unset _clear_typos _t
