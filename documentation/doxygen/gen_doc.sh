#!/usr/bin/env bash
# Version: 2.1
# Date: 2023-04-19
# This bash script generates Ethos-U documentation:
#
# Pre-requisites:
# - bash shell (for Windows: install git for Windows)
# - doxygen 1.17.0

set -o pipefail

# Set version of gen pack library
REQUIRED_GEN_PACK_LIB="0.14.0"

DIRNAME=$(dirname $(readlink -f $0))
GENDIR=../html
REQ_DXY_VERSION="1.17.0"

function usage() {
  echo "Usage: $(basename "$0") [-h] [-s] [-c <comp>]"
  echo " -h,--help               Show usage"
  echo " -s,--no-linkcheck       Skip linkcheck"
  echo " -c,--component <comp>   Select component <comp> to generate documentation for. "
  echo "                         Can be given multiple times. Defaults to all components."
}

while [[ $# -gt 0 ]]; do
  case $1 in
    '-h'|'help')
      usage
      exit 1
    ;;
    '-s'|'--no-linkcheck')
      RUN_LINKCHECKER=0
    ;;
    '-c'|'--component')
      shift
      COMPONENTS+=("$1")
    ;;
    *)
      echo "Invalid command line argument: $1" >&2
      usage
      exit 1
    ;;
  esac
  shift # past argument
done

############ DO NOT EDIT BELOW ###########

# Set GEN_PACK_LIB_PATH to use a specific gen-pack library root
# ... instead of bootstrap based on REQUIRED_GEN_PACK_LIB
if [[ -f "${GEN_PACK_LIB_PATH}/gen-pack" ]]; then
  . "${GEN_PACK_LIB_PATH}/gen-pack"
else
  . <(curl -sL "https://raw.githubusercontent.com/Open-CMSIS-Pack/gen-pack/main/bootstrap")
fi

find_git
find_doxygen "${REQ_DXY_VERSION}"
find_utility "mscgen" "-l | grep 'Mscgen version' | sed -r -e 's/Mscgen version ([^,]+),.*/\1/'" "${REQ_MSCGEN_VERSION}"
[[ ${RUN_LINKCHECKER} != 0 ]] && find_linkchecker

if [ -z "${VERSION_FULL}" ]; then
  VERSION_FULL=$(git_describe "v")
fi

pushd "${DIRNAME}" > /dev/null || exit 1

echo_log "Generating documentation ..."

function generate() {
  if [[ ! (${#COMPONENTS[@]} == 0 || ${COMPONENTS[*]} =~ $1) ]]; then
    return
  fi

  pushd "$1" > /dev/null || exit 1
  
  projectName=$(grep -E "PROJECT_NAME\s+=" "$1.dxy.in" | sed -r -e 's/[^"]*"([^"]+)".*/\1/')
  projectNumberFull="$2"
  if [ -z "${projectNumberFull}" ]; then
    projectNumberFull=$(grep -E "PROJECT_NUMBER\s+=" "$1.dxy.in" | sed -r -e 's/[^"]*"[^0-9]*(([0-9]+\.[0-9]+(\.[0-9]+)?(-.+)?)?)".*/\1/')
  fi
  if [ -z "${projectNumberFull}" ]; then
    projectNumberFull="$(git rev-parse --short HEAD)"
  fi
  projectNumber="${projectNumberFull%+*}"
  datetime=$(date -u +'%a %b %e %Y %H:%M:%S')
  year=$(date -u +'%Y')

  sed -e "s/{projectNumber}/${projectNumber}/" "$1.dxy.in" > "$1.dxy"

  mkdir -p "${DIRNAME}/${GENDIR}/$1/"
  # git_changelog -f html -p "v" > src/history.txt

  echo_log "\"${UTILITY_DOXYGEN}\" \"$1.dxy\""
  "${UTILITY_DOXYGEN}" "$1.dxy"

  mkdir -p "${DIRNAME}/${GENDIR}/$1/search/"
  cp -f "${DIRNAME}/style_template/search.css" "${DIRNAME}/${GENDIR}/$1/search/"

  sed -e "s/{datetime}/${datetime}/" "${DIRNAME}/style_template/footer.js.in" \
    | sed -e "s/{year}/${year}/" \
    | sed -e "s/{projectName}/${projectName}/" \
    | sed -e "s/{projectNumber}/${projectNumber}/" \
    | sed -e "s/{projectNumberFull}/${projectNumberFull}/" \
    > "${DIRNAME}/${GENDIR}/$1/footer.js"

  popd > /dev/null || exit 1
}

echo "Generating documentation ..."
generate "general" "${VERSION_FULL}"
generate "vela"
generate "driver"
generate "integration"
generate "zephyr"


cp -f "${DIRNAME}/index.html" "${DIRNAME}/../html/"

[[ ${RUN_LINKCHECKER} != 0 ]] && check_links --timeout 120 "${DIRNAME}/../html/index.html" "${DIRNAME}"

popd > /dev/null || exit 1

exit 0
