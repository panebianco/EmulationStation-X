#!/usr/bin/env bash
set -euo pipefail

ACTION="${1:-}"
SYSTEM_NAME="${2:-}"
LANG_CODE="${3:-en}"
VIDEOS_ENABLED="${5:-true}"

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
    echo "ERROR: $1" >> "${LOG_FILE}"
    exit 1
}

normalize_lang() {
    case "$1" in
        es|en|fr|de|pt|it|nl|ja|ru) echo "$1" ;;
        *) echo "en" ;;
    esac
}

normalize_videos() {
    case "$1" in
        true|false) echo "$1" ;;
        *) echo "true" ;;
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
    mkdir -p "${SKY_CFG_DIR}"
    touch "${SKY_CFG_FILE}"
}

ensure_system() {
    [[ -n "${SYSTEM_NAME}" ]] || fail "Missing system name"
    [[ -d "${ROMDIR}/${SYSTEM_NAME}" ]] || fail "System folder not found: ${ROMDIR}/${SYSTEM_NAME}"
}

replace_or_add_main_key() {
    local key="$1"
    local value="$2"

    python3 - "$SKY_CFG_FILE" "$key" "$value" <<'PY'
import sys
from pathlib import Path

cfg = Path(sys.argv[1])
key = sys.argv[2]
value = sys.argv[3]

text = cfg.read_text(encoding="utf-8", errors="ignore") if cfg.exists() else ""
if "[main]" not in text:
    text = "[main]\n" + text

lines = text.splitlines()
out = []
in_main = False
done = False

for line in lines:
    stripped = line.strip()

    if stripped.startswith("[") and stripped.endswith("]"):
        if in_main and not done:
            out.append(f'{key}="{value}"')
            done = True
        in_main = (stripped == "[main]")
        out.append(line)
        continue

    if in_main and (stripped.startswith(f"{key}=") or stripped.startswith(f";{key}=")):
        if not done:
            out.append(f'{key}="{value}"')
            done = True
        continue

    out.append(line)

if in_main and not done:
    out.append(f'{key}="{value}"')

cfg.write_text("\n".join(out) + "\n", encoding="utf-8")
PY
}

update_config_lang() {
    local lang="$1"
    local prios="$2"
    replace_or_add_main_key "lang" "${lang}"
    replace_or_add_main_key "langPrios" "${prios}"
}

update_config_videos() {
    local videos="$1"
    replace_or_add_main_key "videos" "${videos}"
}

build_flags() {
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

    : > "${LOG_FILE}"
    {
        echo "ACTION=${ACTION}"
        echo "SYSTEM_NAME=${SYSTEM_NAME}"
        echo "LANG_CODE=${LANG_CODE}"
        echo "VIDEOS_ENABLED=${VIDEOS_ENABLED}"
        echo "SKY_BIN=${SKY_BIN}"
        echo "ROMDIR=${ROMDIR}/${SYSTEM_NAME}"
        echo "-----"
    } >> "${LOG_FILE}"

    echo $$ > "${PID_FILE}"

    case "${ACTION}" in
        gather)
            write_status "running:gather:${SYSTEM_NAME}"
            if run_gather "${flags}" >> "${LOG_FILE}" 2>&1; then
                write_status "done:gather:${SYSTEM_NAME}"
            else
                write_status "error:gather:${SYSTEM_NAME}"
                rm -f "${PID_FILE}"
                exit 1
            fi
            ;;
        generate)
            write_status "running:generate:${SYSTEM_NAME}"
            if run_generate "${flags}" >> "${LOG_FILE}" 2>&1; then
                write_status "done:generate:${SYSTEM_NAME}"
            else
                write_status "error:generate:${SYSTEM_NAME}"
                rm -f "${PID_FILE}"
                exit 1
            fi
            ;;
        *)
            rm -f "${PID_FILE}"
            fail "Unknown action: ${ACTION}"
            ;;
    esac

    rm -f "${PID_FILE}"
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
    VIDEOS_ENABLED="$(normalize_videos "${VIDEOS_ENABLED}")"

    update_config_lang "${LANG_CODE}" "$(build_lang_prios "${LANG_CODE}")"
    update_config_videos "${VIDEOS_ENABLED}"

    run_foreground
}

main "$@"