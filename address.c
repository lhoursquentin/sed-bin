#include <assert.h>
#include <regex.h>
#include <stdbool.h>
#include <stddef.h>

#include "address.h"
#include "status.h"

/*
 * How to handle ranges is a bit subjective, the POSIX spec doesn't say much.
 * For instance if we're looping without reading any new input, should the
 * range match again (assuming the range addresses are still matching)?
 * GNU sed discards matches only once when the exit condition is reached, even
 * in a loop, why would a numeric range stop matching if we're still on the same
 * line? Probably because that's the only way with regexes.
 *
 * The current implementation only uses a suppress list in the <line-nb>,/regex/
 * address because it's the only way to make it work for this, I feel like for
 * the other combinations it makes more sense to allow to match again, even if
 * it is less consistent. Implicitly /regex/,/regex/ will never match twice in
 * the end case as well.
 *
 * Should a range number end once it is equal (similarly to regex addresses) or
 * once it goes over? What should 2!{1,2=} print for line 3,4,5... of input?
 * busybox seems to have a bug there, we'll follow the GNU way, that is check if
 * we went over.
*/

bool addr_nn(
  Status *const status,
  const size_t start,
  const size_t end,
  const size_t id
) {
  const size_t line_nb = status->line_nb;
  size_t *const range_ids = status->range_ids;
  size_t *free_slot = NULL;
  size_t i;
  for (i = 0; i < MAX_ACTIVE_RANGES; ++i) {
    if (range_ids[i] == id) {
      break;
    } else if (free_slot == NULL && range_ids[i] == 0) {
      free_slot = range_ids + i;
    }
  }
  if (i == MAX_ACTIVE_RANGES) {
    // Could not find active range, let's check if we can start a new one
    if (line_nb == start || (line_nb >= start && line_nb <= end)) {
      if (line_nb < end) {
        assert(free_slot);
        *free_slot = id;
      }
      return true;
    }
  } else {
    // inside active range, need to check if we can free it.
    if (line_nb >= end) {
      range_ids[i] = 0;
    }
    return line_nb <= end;
  }
  return false;
}

bool addr_nr(
  Status *const status,
  const size_t start,
  Regex *const end,
  const size_t id
) {
  /*
   * Since we systematically match if line nb >= start we need to remember if we
   * reached the end address, which is the task of the suppressed array.
  */
  const size_t line_nb = status->line_nb;
  size_t *const range_ids = status->range_ids;
  size_t *const suppressed_range_ids = status->suppressed_range_ids;
  size_t *free_slot = NULL;
  size_t i;
  for (i = 0; i < MAX_ACTIVE_RANGES; ++i) {
    if (suppressed_range_ids[i] == id) {
      return false;
    } else if (range_ids[i] == id) {
      break;
    } else if (free_slot == NULL && range_ids[i] == 0) {
      free_slot = range_ids + i;
    }
  }
  if (i == MAX_ACTIVE_RANGES) {
    // Could not find active range, let's check if we can start a new one
    if (line_nb >= start) {
      if (addr_r(status, end)) {
        suppressed_range_ids[i] = id;
      } else {
        assert(free_slot);
        *free_slot = id;
      }
      return true;
    }
  } else {
    // inside active regex range, need to check if we can free it.
    if (addr_r(status, end)) {
      suppressed_range_ids[i] = id;
      range_ids[i] = 0;
    }
    return true;
  }
  return false;
}

bool addr_rn(
  Status *const status,
  Regex *const start,
  const size_t end,
  const size_t id
) {
  const size_t line_nb = status->line_nb;
  size_t *const range_ids = status->range_ids;
  size_t *free_slot = NULL;
  size_t i;
  for (i = 0; i < MAX_ACTIVE_RANGES; ++i) {
    if (range_ids[i] == id) {
      break;
    } else if (free_slot == NULL && range_ids[i] == 0) {
      free_slot = range_ids + i;
    }
  }
  if (i == MAX_ACTIVE_RANGES) {
    // Could not find active range, let's check if we can start a new one
    if (addr_r(status, start)) {
      if (line_nb < end) {
        assert(free_slot);
        *free_slot = id;
      }
      return true;
    }
  } else {
    // inside active range, need to check if we can free it.
    if (line_nb >= end) {
      range_ids[i] = 0;
    }
    return line_nb <= end;
  }
  return false;
}

bool addr_rr(
  Status *const status,
  Regex *const start,
  Regex *const end,
  const size_t id
) {
  size_t *const range_ids = status->range_ids;
  size_t *free_slot = NULL;
  size_t i;
  for (i = 0; i < MAX_ACTIVE_RANGES; ++i) {
    if (range_ids[i] == id) {
      break;
    } else if (free_slot == NULL && range_ids[i] == 0) {
      free_slot = range_ids + i;
    }
  }
  if (i == MAX_ACTIVE_RANGES) {
    // Could not find active range, let's check if we can start a new one
    if (addr_r(status, start)) {
      assert(free_slot);
      *free_slot = id;
      return true;
    }
  } else {
    // inside active range, need to check if we can free it.
    if (addr_r(status, end)) {
      range_ids[i] = 0;
    }
    return true;
  }
  return false;
}

bool addr_n(const Status *const status, const size_t line_nb) {
  return status->line_nb == line_nb;
}

bool addr_r(Status *const status, Regex *const regex) {
  status->last_regex = regex;
  regex_t *const regex_obj = &regex->obj;

  if (!regex->compiled) {
    assert(regcomp(regex_obj, regex->str, 0) == 0);
    regex->compiled = true;
  }

  const char *const pattern_space = status->pattern_space;

  return !regexec(regex_obj, pattern_space, 0, NULL, 0);
}
