#!/bin/sh
set -u

__usage() {
  cat << EOF
An implementation of sed based on C translation.

Usage:

  sed [-n] script [file...]

  sed [-n] -e script [-e script]... [-f script_file]... [file...]

  sed [-n] [-e script]... -f script_file [-f script_file]...  [file...]
EOF
  exit "${1-0}"
}

__sed_parse_args() {
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
        __usage 1 >&2
      fi

      shift; nb_args="$((nb_args - 1))"
      script="$script
$1"
      ;;
    -f)
      f_opt_found=true
      shift; nb_args="$((nb_args - 1))"
      script="$script
$(cat "$1")"
      ;;
    -h|--help)
      __usage
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

__sed_main "$@"
}

__sed_main() { __sed_default_main "$@"; }
__sed_default_main() {
: "${SED_DIR:=${0%/*}/}"
mydir=$SED_DIR
case "$mydir" in
  /*) ;;
  *) mydir=$PWD/$mydir ;;
esac

bin="${BIN-$mydir/sed-bin}"
default_translator=$mydir/par.sed
translator="${SED_TRANSLATOR-$default_translator}"
generated_file=$mydir/generated.c
case "$bin" in
  /*) ;;
  *) bin=$mydir/$bin ;;
esac

if "$e_opt_found" || "$f_opt_found"; then
  # delete extra leading newline, this is important for #n handling
  script="${script#?}"
elif ! "$no_opt_script_found"; then
  __usage 1 >&2
fi

  __sed_make "$script" && __sed_exec "$@"
}

__sed_make() { __sed_default_make "$@"; }
__sed_default_make() {  # args: script
printf '%s\n' "$1" | "$translator" > "$generated_file" &&
  make -C "$mydir" -s
}
__sed_exec() { __sed_default_exec "$@"; }
__sed_default_exec() {
  case $# in
    0) ;;
    1) exec <"$1"; shift ;;  # will lose filename, but we always do anyway
    *) cat "$@" | __sed_default_exec ;;
  esac
  if "$n_opt_found"; then
    set -- -n
  fi
  exec "$bin" "$@"
}

if [ -z "${SED_LIBMODE:-}" ]; then
  __sed_parse_args "$@"
else
  eval "$SED_LIBMODE"
fi
