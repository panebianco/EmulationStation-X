#!/usr/bin/env bash
set -euo pipefail

ACTION="${1:-}"
SYSTEM_NAME="${2:-}"
LANG_CODE="${3:-en}"

HOME_DIR="${HOME}"
ROMDIR="${HOME_DIR}/RetroPie/roms"
SKY_BIN="$(command -v Skyscraper || true)"
SKY_CFG_DIR="${HOME_DIR}/.skyscraper"
SKY_CFG_FILE="${SKY_CFG_DIR}/config.ini"

STATE_DIR="/tmp/esx-skyscraper"
PID_FILE="${STATE_DIR}/pid"
STATUS_FILE="${STATE_DIR}/status"
LOG_FILE="${STATE_DIR}/log"

mkdir -p "${STATE_DIR}"

write_status() {
    printf '%s\n' "$1" > "${STATUS_FILE}"
}

fail() {
    write_status "error:$1"
    exit 1
}

normalize_lang() {
    case "$1" in
        es|en|fr|de|pt|it|nl|ja|ru) echo "$1" ;;
        *) echo "en" ;;
    esac
}

build_lang_prios() {
    local lang="$1"
    if [[ "$lang" == "en" ]]; then
        echo "en"
    else
        echo "${lang},en"
    fi
}

ensure_bin() {
    [[ -n "${SKY_BIN}" ]] || fail "Skyscraper not found in PATH"
}

ensure_config() {
    [[ -f "${SKY_CFG_FILE}" ]] || fail "Missing config: ${SKY_CFG_FILE}"
}

ensure_system() {
    [[ -n "${SYSTEM_NAME}" ]] || fail "Missing system name"
    [[ -d "${ROMDIR}/${SYSTEM_NAME}" ]] || fail "System folder not found: ${ROMDIR}/${SYSTEM_NAME}"
}

update_config_lang() {
    local lang="$1"
    local prios="$2"

    mkdir -p "${SKY_CFG_DIR}"
    touch "${SKY_CFG_FILE}"

    python3 - "$SKY_CFG_FILE" "$lang" "$prios" <<'PY'
import sys
from pathlib import Path

cfg = Path(sys.argv[1])
lang = sys.argv[2]
prios = sys.argv[3]

text = cfg.read_text(encoding="utf-8", errors="ignore") if cfg.exists() else ""

if "[main]" not in text:
    text = "[main]\n" + text

lines = text.splitlines()
out = []
in_main = False
lang_done = False
prios_done = False

for line in lines:
    stripped = line.strip()

    if stripped.startswith("[") and stripped.endswith("]"):
        if in_main and not lang_done:
            out.append(f'lang="{lang}"')
            lang_done = True
        if in_main and not prios_done:
            out.append(f'langPrios="{prios}"')
            prios_done = True
        in_main = (stripped == "[main]")
        out.append(line)
        continue

    if in_main and (stripped.startswith("lang=") or stripped.startswith(";lang=")):
        if not lang_done:
            out.append(f'lang="{lang}"')
            lang_done = True
        continue

    if in_main and (stripped.startswith("langPrios=") or stripped.startswith(";langPrios=")):
        if not prios_done:
            out.append(f'langPrios="{prios}"')
            prios_done = True
        continue

    out.append(line)

if in_main and not lang_done:
    out.append(f'lang="{lang}"')
if in_main and not prios_done:
    out.append(f'langPrios="{prios}"')

cfg.write_text("\n".join(out) + "\n", encoding="utf-8")
PY
}

build_flags() {
    # Primera versión simple y segura
    echo "unattend,skipped,relative"
}

run_gather() {
    local flags="$1"
    "${SKY_BIN}" \
        -p "${SYSTEM_NAME}" \
        -g "${ROMDIR}/${SYSTEM_NAME}" \
        -o "${ROMDIR}/${SYSTEM_NAME}/media" \
        -s screenscraper \
        --lang "${LANG_CODE}" \
        --flags "${flags}"
}

run_generate() {
    local flags="$1"
    "${SKY_BIN}" \
        -p "${SYSTEM_NAME}" \
        -g "${ROMDIR}/${SYSTEM_NAME}" \
        -o "${ROMDIR}/${SYSTEM_NAME}/media" \
        --lang "${LANG_CODE}" \
        --flags "${flags}"
}

run_foreground() {
    local flags
    flags="$(build_flags)"

    case "${ACTION}" in
        gather)
            write_status "running:gather:${SYSTEM_NAME}"
            run_gather "${flags}" >> "${LOG_FILE}" 2>&1
            write_status "done:gather:${SYSTEM_NAME}"
            ;;
        generate)
            write_status "running:generate:${SYSTEM_NAME}"
            run_generate "${flags}" >> "${LOG_FILE}" 2>&1
            write_status "done:generate:${SYSTEM_NAME}"
            ;;
        *)
            fail "Unknown action: ${ACTION}"
            ;;
    esac
}

run_background() {
    : > "${LOG_FILE}"
    write_status "starting:${ACTION}:${SYSTEM_NAME}"

    (
        echo $$ > "${PID_FILE}"
        run_foreground
        rm -f "${PID_FILE}"
    ) &
}

status_only() {
    if [[ -f "${STATUS_FILE}" ]]; then
        cat "${STATUS_FILE}"
    else
        echo "idle"
    fi
}

stop_job() {
    if [[ -f "${PID_FILE}" ]]; then
        local pid
        pid="$(cat "${PID_FILE}")"
        if kill -0 "${pid}" 2>/dev/null; then
            kill "${pid}" 2>/dev/null || true
            write_status "stopped"
        fi
        rm -f "${PID_FILE}"
    fi
}

main() {
    case "${ACTION}" in
        status)
            status_only
            exit 0
            ;;
        stop)
            stop_job
            exit 0
            ;;
    esac

    ensure_bin
    ensure_config
    ensure_system

    LANG_CODE="$(normalize_lang "${LANG_CODE}")"
    update_config_lang "${LANG_CODE}" "$(build_lang_prios "${LANG_CODE}")"

    run_background
    echo "started:${ACTION}:${SYSTEM_NAME}"
}

main "$@"