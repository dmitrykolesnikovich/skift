#pragma once

#include <karm-base/range.h>
#include <karm-base/string.h>
#include <karm-base/vec.h>
#include <karm-debug/logger.h>

#include "spec.h"

template <>
struct Karm::Fmt::Formatter<Handover::Record> {
    Result<size_t> format(Io::_TextWriter &writer, Handover::Record record) {
        return Fmt::format(writer, "Record({}, {x}-{x})", record.name(), record.start, record.end());
    }
};

namespace Handover {

inline USizeRange rangeOf(Record record) {
    return {record.start, record.size};
}

inline bool colidesWith(Record record, Record other) {
    return rangeOf(record).overlaps(rangeOf(other));
}

inline Cons<Record, Record> split(Record record, Record other) {
    auto [lower, upper] = rangeOf(record).split(rangeOf(other));
    Record lowerRecord = record;
    Record upperRecord = record;
    lowerRecord.start = lower.start;
    lowerRecord.size = lower.size;
    upperRecord.start = upper.start;
    upperRecord.size = upper.size;
    return {lowerRecord, upperRecord};
}

struct Builder {
    void *_buf{};
    size_t _size{};
    char *_string{};
    _Vec<ViewBuf<Record>> _records;

    Builder(MutSlice<Byte> slice)
        : _buf(slice.buf()),
          _size(slice.len()),
          _string((char *)end(slice)),
          _records(ViewBuf<Record>{(Inert<Record> *)payload().records, _size / sizeof(Record)}) {
        payload() = {};
        payload().magic = COOLBOOT;
        payload().size = slice.len();
    }

    Payload &payload() {
        return *static_cast<Payload *>(_buf);
    }

    void add(Record record) {
        if (record.size == 0)
            return;

        for (size_t i = 0; i < _records.len(); i++) {
            auto other = _records[i];

            // Merge with previous
            if (other.tag == record.tag &&
                other.end() == record.start &&
                shouldMerge(record.tag)) {
                // Debug::ldebug("handover: merge {} with {}", record, other);
                _records.removeAt(i);
                other.size += record.size;
                add(other);
                return;
            }

            // Merge with next
            if (other.tag == record.tag &&
                other.start == record.end() &&
                shouldMerge(record.tag)) {
                // Debug::ldebug("handover: merge {} with {}", record, other);

                _records.removeAt(i);
                record.size += other.size;
                add(record);
                return;
            }

            if (colidesWith(record, other)) {
                if (shouldMerge(record.tag) && !shouldMerge(other.tag)) {
                    // Debug::ldebug("handover: splitting record {} with {}", record, other);

                    _records.removeAt(i);
                    auto [lower, upper] = split(record, other);

                    add(other);
                    add(lower);
                    add(upper);
                    return;
                } else if (!shouldMerge(record.tag) && shouldMerge(other.tag)) {
                    // Debug::ldebug("handover: splitting record {} with {}", other, record);

                    _records.removeAt(i);

                    auto [lower, upper] = split(other, record);
                    add(record);
                    add(lower);
                    add(upper);
                    return;
                } else {
                    Debug::lwarn("handover: record {} colides with {}", record, other);
                    return;
                }
            }

            if (other.start > record.start) {
                // Debug::ldebug("handover: insert {} at {}", record, i);
                _records.insert(i, record);
                return;
            }
        }

        // Debug::ldebug("handover: append {}", record);
        _records.pushBack(record);
    }

    void add(Tag tag, uint32_t flags = 0, USizeRange range = {}, uint64_t more = 0) {
        add({
            .tag = tag,
            .flags = flags,
            .start = range.start,
            .size = range.size,
            .more = more,
        });
    }

    size_t add(Str str) {
        _string -= str.len() + 1;
        memcpy(_string, str.buf(), str.len());
        _string[str.len()] = '\0';
        return _string - static_cast<char *>(_buf);
    }

    void agent(Str str) {
        payload().agent = add(str);
    }

    Payload &finalize() {
        payload().len = _records.len();
        return payload();
    }
};

} // namespace Handover
