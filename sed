#!/bin/sh

pwd0=$PWD
cd -P -- "${0%/*}/"

usage() {
  basename="${0##*/}"
  cat << EOF
An implementation of sed based on C translation.

Usage:

  sed [-n] script [file...]

  sed [-n] -e script [-e script]... [-f script_file]... [file...]

  sed [-n] [-e script]... -f script_file [-f script_file]...  [file...]
EOF
  exit "${1-0}"
}

bin="${BIN-./sed-bin}"
default_translator=./par.sed
translator="${SED_TRANSLATOR-$default_translator}"
generated_file=generated.c

nb_args="$#"
e_opt_found=false
f_opt_found=false
n_opt_found=false
no_opt_script_found=false
script=
while [ "$nb_args" -gt 0 ]; do
  case "$1" in
    -e)
      e_opt_found=true
      if "$f_opt_found"; then
        usage 1 >&2
      fi

      shift; nb_args="$((nb_args - 1))"
      script="$script
$1"
      ;;
    -f)
      f_opt_found=true
      shift; nb_args="$((nb_args - 1))"
      script="$script
$(cd "$pwd0"; cat "$1")"
      ;;
    -h|--help)
      usage
      ;;
    -n)
      n_opt_found=true
      ;;
    *)
      if "$e_opt_found" || "$f_opt_found"; then
        break
      else
        no_opt_script_found=true
        script="$1"
        shift
        break
      fi
      ;;
  esac
  shift; nb_args="$((nb_args - 1))"
done

if "$e_opt_found" || "$f_opt_found"; then
  # delete extra leading newline, this is important for #n handling
  script="${script#?}"
elif ! "$no_opt_script_found"; then
  usage 1 >&2
fi

printf '%s\n' "$script" | "$translator" > "$generated_file" &&
  make -s &&
  cat "$@" | {
    set --
    if "$n_opt_found"; then
      set -- -n
    fi
    case "$bin" in
      /*)
        set -- "$bin" "$@"
        ;;
      *)
        set -- ./"$bin" "$@"
        ;;
    esac
    "$@"
  }
