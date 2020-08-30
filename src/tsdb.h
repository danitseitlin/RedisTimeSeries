/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/
#ifndef TSDB_H
#define TSDB_H

#include "redismodule.h"
#include "compaction.h"
#include "consts.h"
#include "indexer.h"
#include "generic_chunk.h"

typedef struct CompactionRule {
    RedisModuleString *destKey;
    timestamp_t timeBucket;
    AggregationClass *aggClass;
    int aggType;
    void *aggContext;
    struct CompactionRule *nextRule;
    timestamp_t startCurrentTimeBucket;
} CompactionRule;

typedef struct Series {
    RedisModuleDict* chunks;
    Chunk_t *lastChunk;
    uint64_t retentionTime;
    short maxSamplesPerChunk;
    short options;
    CompactionRule *rules;
    timestamp_t lastTimestamp;
    double lastValue;
    Label *labels;
    RedisModuleString *keyName;
    size_t labelsCount;
    RedisModuleString *srcKey;
    ChunkFuncs *funcs;
    size_t totalSamples;
} Series;

typedef struct SeriesIterator {
    Series *series;
    RedisModuleDictIter *dictIter;
    Chunk_t *currentChunk;
    int chunkIteratorInitialized;
    ChunkIter_t *chunkIterator;
    api_timestamp_t maxTimestamp;
    api_timestamp_t minTimestamp;
} SeriesIterator;

Series *NewSeries(RedisModuleString *keyName, Label *labels, size_t labelsCount,
                uint64_t retentionTime, short maxSamplesPerChunk, int uncompressed);
void FreeSeries(void *value);
void CleanLastDeletedSeries(RedisModuleCtx *ctx, RedisModuleString *key);
void FreeCompactionRule(void *value);
size_t SeriesMemUsage(const void *value);
int SeriesAddSample(Series *series, api_timestamp_t timestamp, double value);
int SeriesDeleteRule(Series *series, RedisModuleString *destKey);
int SeriesSetSrcRule(Series *series, RedisModuleString *srctKey);
int SeriesDeleteSrcRule(Series *series, RedisModuleString *srctKey);

CompactionRule *SeriesAddRule(Series *series, RedisModuleString *destKeyStr, int aggType, uint64_t timeBucket);
int SeriesCreateRulesFromGlobalConfig(RedisModuleCtx *ctx, RedisModuleString *keyName, Series *series, Label *labels, size_t labelsCount);
size_t SeriesGetNumSamples(const Series *series);

// Iterator over the series
SeriesIterator SeriesQuery(Series *series, api_timestamp_t minTimestamp, api_timestamp_t maxTimestamp);
int SeriesIteratorGetNext(SeriesIterator *iterator, Sample *currentSample);
void SeriesIteratorClose(SeriesIterator *iterator);

// return first timestamp in retention window, and set `skipped` to number of samples outside of retention
timestamp_t getFirstValidTimestamp(Series *series, long long *skipped);

CompactionRule *NewRule(RedisModuleString *destKey, int aggType, uint64_t timeBucket);
#endif /* TSDB_H */
