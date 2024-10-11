/* THIS FILE IS GENERATED BY gen_metrics.py. DO NOT HAND EDIT. */

#include "../fd_metrics_base.h"

#define FD_METRICS_HISTOGRAM_PACK_SCHEDULE_MICROBLOCK_DURATION_SECONDS_OFF  (176UL)
#define FD_METRICS_HISTOGRAM_PACK_SCHEDULE_MICROBLOCK_DURATION_SECONDS_NAME "pack_schedule_microblock_duration_seconds"
#define FD_METRICS_HISTOGRAM_PACK_SCHEDULE_MICROBLOCK_DURATION_SECONDS_TYPE (FD_METRICS_TYPE_HISTOGRAM)
#define FD_METRICS_HISTOGRAM_PACK_SCHEDULE_MICROBLOCK_DURATION_SECONDS_DESC "Duration of scheduling one microblock"
#define FD_METRICS_HISTOGRAM_PACK_SCHEDULE_MICROBLOCK_DURATION_SECONDS_MIN  (1e-08)
#define FD_METRICS_HISTOGRAM_PACK_SCHEDULE_MICROBLOCK_DURATION_SECONDS_MAX  (0.1)
#define FD_METRICS_HISTOGRAM_PACK_SCHEDULE_MICROBLOCK_DURATION_SECONDS_CVT  (FD_METRICS_CONVERTER_SECONDS)

#define FD_METRICS_HISTOGRAM_PACK_INSERT_TRANSACTION_DURATION_SECONDS_OFF  (193UL)
#define FD_METRICS_HISTOGRAM_PACK_INSERT_TRANSACTION_DURATION_SECONDS_NAME "pack_insert_transaction_duration_seconds"
#define FD_METRICS_HISTOGRAM_PACK_INSERT_TRANSACTION_DURATION_SECONDS_TYPE (FD_METRICS_TYPE_HISTOGRAM)
#define FD_METRICS_HISTOGRAM_PACK_INSERT_TRANSACTION_DURATION_SECONDS_DESC "Duration of inserting one transaction into the pool of available transactions"
#define FD_METRICS_HISTOGRAM_PACK_INSERT_TRANSACTION_DURATION_SECONDS_MIN  (1e-08)
#define FD_METRICS_HISTOGRAM_PACK_INSERT_TRANSACTION_DURATION_SECONDS_MAX  (0.1)
#define FD_METRICS_HISTOGRAM_PACK_INSERT_TRANSACTION_DURATION_SECONDS_CVT  (FD_METRICS_CONVERTER_SECONDS)

#define FD_METRICS_HISTOGRAM_PACK_TOTAL_TRANSACTIONS_PER_MICROBLOCK_COUNT_OFF  (210UL)
#define FD_METRICS_HISTOGRAM_PACK_TOTAL_TRANSACTIONS_PER_MICROBLOCK_COUNT_NAME "pack_total_transactions_per_microblock_count"
#define FD_METRICS_HISTOGRAM_PACK_TOTAL_TRANSACTIONS_PER_MICROBLOCK_COUNT_TYPE (FD_METRICS_TYPE_HISTOGRAM)
#define FD_METRICS_HISTOGRAM_PACK_TOTAL_TRANSACTIONS_PER_MICROBLOCK_COUNT_DESC "Count of transactions in a scheduled microblock, including both votes and non-votes"
#define FD_METRICS_HISTOGRAM_PACK_TOTAL_TRANSACTIONS_PER_MICROBLOCK_COUNT_MIN  (0UL)
#define FD_METRICS_HISTOGRAM_PACK_TOTAL_TRANSACTIONS_PER_MICROBLOCK_COUNT_MAX  (64UL)
#define FD_METRICS_HISTOGRAM_PACK_TOTAL_TRANSACTIONS_PER_MICROBLOCK_COUNT_CVT  (FD_METRICS_CONVERTER_NONE)

#define FD_METRICS_HISTOGRAM_PACK_VOTES_PER_MICROBLOCK_COUNT_OFF  (227UL)
#define FD_METRICS_HISTOGRAM_PACK_VOTES_PER_MICROBLOCK_COUNT_NAME "pack_votes_per_microblock_count"
#define FD_METRICS_HISTOGRAM_PACK_VOTES_PER_MICROBLOCK_COUNT_TYPE (FD_METRICS_TYPE_HISTOGRAM)
#define FD_METRICS_HISTOGRAM_PACK_VOTES_PER_MICROBLOCK_COUNT_DESC "Count of simple vote transactions in a scheduled microblock"
#define FD_METRICS_HISTOGRAM_PACK_VOTES_PER_MICROBLOCK_COUNT_MIN  (0UL)
#define FD_METRICS_HISTOGRAM_PACK_VOTES_PER_MICROBLOCK_COUNT_MAX  (64UL)
#define FD_METRICS_HISTOGRAM_PACK_VOTES_PER_MICROBLOCK_COUNT_CVT  (FD_METRICS_CONVERTER_NONE)

#define FD_METRICS_COUNTER_PACK_NORMAL_TRANSACTION_RECEIVED_OFF  (244UL)
#define FD_METRICS_COUNTER_PACK_NORMAL_TRANSACTION_RECEIVED_NAME "pack_normal_transaction_received"
#define FD_METRICS_COUNTER_PACK_NORMAL_TRANSACTION_RECEIVED_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_NORMAL_TRANSACTION_RECEIVED_DESC "Count of transactions received via the normal TPU path"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_OFF  (245UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_CNT  (15UL)

#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_BUNDLE_BLACKLIST_OFF  (245UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_BUNDLE_BLACKLIST_NAME "pack_transaction_inserted_bundle_blacklist"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_BUNDLE_BLACKLIST_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_BUNDLE_BLACKLIST_DESC "Result of inserting a transaction into the pack object (Transaction uses an account on the bundle blacklist)"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_WRITE_SYSVAR_OFF  (246UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_WRITE_SYSVAR_NAME "pack_transaction_inserted_write_sysvar"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_WRITE_SYSVAR_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_WRITE_SYSVAR_DESC "Result of inserting a transaction into the pack object (Transaction tries to write to a sysvar)"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_ESTIMATION_FAIL_OFF  (247UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_ESTIMATION_FAIL_NAME "pack_transaction_inserted_estimation_fail"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_ESTIMATION_FAIL_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_ESTIMATION_FAIL_DESC "Result of inserting a transaction into the pack object (Estimating compute cost and/or fee failed)"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_DUPLICATE_ACCOUNT_OFF  (248UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_DUPLICATE_ACCOUNT_NAME "pack_transaction_inserted_duplicate_account"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_DUPLICATE_ACCOUNT_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_DUPLICATE_ACCOUNT_DESC "Result of inserting a transaction into the pack object (Transaction included an account address twice)"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_TOO_MANY_ACCOUNTS_OFF  (249UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_TOO_MANY_ACCOUNTS_NAME "pack_transaction_inserted_too_many_accounts"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_TOO_MANY_ACCOUNTS_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_TOO_MANY_ACCOUNTS_DESC "Result of inserting a transaction into the pack object (Transaction tried to load too many accounts)"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_TOO_LARGE_OFF  (250UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_TOO_LARGE_NAME "pack_transaction_inserted_too_large"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_TOO_LARGE_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_TOO_LARGE_DESC "Result of inserting a transaction into the pack object (Transaction requests too many CUs)"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_EXPIRED_OFF  (251UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_EXPIRED_NAME "pack_transaction_inserted_expired"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_EXPIRED_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_EXPIRED_DESC "Result of inserting a transaction into the pack object (Transaction already expired)"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_ADDR_LUT_OFF  (252UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_ADDR_LUT_NAME "pack_transaction_inserted_addr_lut"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_ADDR_LUT_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_ADDR_LUT_DESC "Result of inserting a transaction into the pack object (Transaction loaded accounts from a lookup table)"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_UNAFFORDABLE_OFF  (253UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_UNAFFORDABLE_NAME "pack_transaction_inserted_unaffordable"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_UNAFFORDABLE_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_UNAFFORDABLE_DESC "Result of inserting a transaction into the pack object (Fee payer's balance below transaction fee)"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_DUPLICATE_OFF  (254UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_DUPLICATE_NAME "pack_transaction_inserted_duplicate"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_DUPLICATE_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_DUPLICATE_DESC "Result of inserting a transaction into the pack object (Pack aware of transaction with same signature)"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_PRIORITY_OFF  (255UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_PRIORITY_NAME "pack_transaction_inserted_priority"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_PRIORITY_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_PRIORITY_DESC "Result of inserting a transaction into the pack object (Transaction's fee was too low given its compute unit requirement and other competing transactions)"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_NONVOTE_ADD_OFF  (256UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_NONVOTE_ADD_NAME "pack_transaction_inserted_nonvote_add"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_NONVOTE_ADD_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_NONVOTE_ADD_DESC "Result of inserting a transaction into the pack object (Transaction that was not a simple vote added to pending transactions)"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_VOTE_ADD_OFF  (257UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_VOTE_ADD_NAME "pack_transaction_inserted_vote_add"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_VOTE_ADD_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_VOTE_ADD_DESC "Result of inserting a transaction into the pack object (Simple vote transaction was added to pending transactions)"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_NONVOTE_REPLACE_OFF  (258UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_NONVOTE_REPLACE_NAME "pack_transaction_inserted_nonvote_replace"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_NONVOTE_REPLACE_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_NONVOTE_REPLACE_DESC "Result of inserting a transaction into the pack object (Transaction that was not a simple vote replaced a lower priority transaction)"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_VOTE_REPLACE_OFF  (259UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_VOTE_REPLACE_NAME "pack_transaction_inserted_vote_replace"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_VOTE_REPLACE_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_VOTE_REPLACE_DESC "Result of inserting a transaction into the pack object (Simple vote transaction replaced a lower priority transaction)"

#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_OFF  (260UL)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_CNT  (16UL)

#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_NO_BANK_NO_LEADER_NO_MICROBLOCK_OFF  (260UL)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_NO_BANK_NO_LEADER_NO_MICROBLOCK_NAME "pack_metric_timing_no_txn_no_bank_no_leader_no_microblock"
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_NO_BANK_NO_LEADER_NO_MICROBLOCK_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_NO_BANK_NO_LEADER_NO_MICROBLOCK_DESC "Time in nanos spent in each state (Pack had no transactions available, and wasn't leader)"

#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_NO_BANK_NO_LEADER_NO_MICROBLOCK_OFF  (261UL)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_NO_BANK_NO_LEADER_NO_MICROBLOCK_NAME "pack_metric_timing_txn_no_bank_no_leader_no_microblock"
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_NO_BANK_NO_LEADER_NO_MICROBLOCK_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_NO_BANK_NO_LEADER_NO_MICROBLOCK_DESC "Time in nanos spent in each state (Pack had transactions available, but wasn't leader or had hit a limit)"

#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_BANK_NO_LEADER_NO_MICROBLOCK_OFF  (262UL)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_BANK_NO_LEADER_NO_MICROBLOCK_NAME "pack_metric_timing_no_txn_bank_no_leader_no_microblock"
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_BANK_NO_LEADER_NO_MICROBLOCK_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_BANK_NO_LEADER_NO_MICROBLOCK_DESC "Time in nanos spent in each state (Pack had no transactions available, had banks but wasn't leader?)"

#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_BANK_NO_LEADER_NO_MICROBLOCK_OFF  (263UL)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_BANK_NO_LEADER_NO_MICROBLOCK_NAME "pack_metric_timing_txn_bank_no_leader_no_microblock"
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_BANK_NO_LEADER_NO_MICROBLOCK_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_BANK_NO_LEADER_NO_MICROBLOCK_DESC "Time in nanos spent in each state (Pack had transactions available, had banks but wasn't leader?)"

#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_NO_BANK_LEADER_NO_MICROBLOCK_OFF  (264UL)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_NO_BANK_LEADER_NO_MICROBLOCK_NAME "pack_metric_timing_no_txn_no_bank_leader_no_microblock"
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_NO_BANK_LEADER_NO_MICROBLOCK_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_NO_BANK_LEADER_NO_MICROBLOCK_DESC "Time in nanos spent in each state (Pack had no transactions available, and was leader but had no available banks)"

#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_NO_BANK_LEADER_NO_MICROBLOCK_OFF  (265UL)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_NO_BANK_LEADER_NO_MICROBLOCK_NAME "pack_metric_timing_txn_no_bank_leader_no_microblock"
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_NO_BANK_LEADER_NO_MICROBLOCK_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_NO_BANK_LEADER_NO_MICROBLOCK_DESC "Time in nanos spent in each state (Pack had transactions available, was leader, but had no available banks)"

#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_BANK_LEADER_NO_MICROBLOCK_OFF  (266UL)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_BANK_LEADER_NO_MICROBLOCK_NAME "pack_metric_timing_no_txn_bank_leader_no_microblock"
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_BANK_LEADER_NO_MICROBLOCK_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_BANK_LEADER_NO_MICROBLOCK_DESC "Time in nanos spent in each state (Pack had available banks but no transactions)"

#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_BANK_LEADER_NO_MICROBLOCK_OFF  (267UL)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_BANK_LEADER_NO_MICROBLOCK_NAME "pack_metric_timing_txn_bank_leader_no_microblock"
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_BANK_LEADER_NO_MICROBLOCK_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_BANK_LEADER_NO_MICROBLOCK_DESC "Time in nanos spent in each state (Pack had banks and transactions available but couldn't schedule anything non-conflicting)"

#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_NO_BANK_NO_LEADER_MICROBLOCK_OFF  (268UL)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_NO_BANK_NO_LEADER_MICROBLOCK_NAME "pack_metric_timing_no_txn_no_bank_no_leader_microblock"
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_NO_BANK_NO_LEADER_MICROBLOCK_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_NO_BANK_NO_LEADER_MICROBLOCK_DESC "Time in nanos spent in each state (Pack scheduled a non-empty microblock while not leader?)"

#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_NO_BANK_NO_LEADER_MICROBLOCK_OFF  (269UL)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_NO_BANK_NO_LEADER_MICROBLOCK_NAME "pack_metric_timing_txn_no_bank_no_leader_microblock"
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_NO_BANK_NO_LEADER_MICROBLOCK_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_NO_BANK_NO_LEADER_MICROBLOCK_DESC "Time in nanos spent in each state (Pack scheduled a non-empty microblock while not leader?)"

#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_BANK_NO_LEADER_MICROBLOCK_OFF  (270UL)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_BANK_NO_LEADER_MICROBLOCK_NAME "pack_metric_timing_no_txn_bank_no_leader_microblock"
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_BANK_NO_LEADER_MICROBLOCK_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_BANK_NO_LEADER_MICROBLOCK_DESC "Time in nanos spent in each state (Pack scheduled a non-empty microblock while not leader?)"

#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_BANK_NO_LEADER_MICROBLOCK_OFF  (271UL)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_BANK_NO_LEADER_MICROBLOCK_NAME "pack_metric_timing_txn_bank_no_leader_microblock"
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_BANK_NO_LEADER_MICROBLOCK_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_BANK_NO_LEADER_MICROBLOCK_DESC "Time in nanos spent in each state (Pack scheduled a non-empty microblock while not leader?)"

#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_NO_BANK_LEADER_MICROBLOCK_OFF  (272UL)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_NO_BANK_LEADER_MICROBLOCK_NAME "pack_metric_timing_no_txn_no_bank_leader_microblock"
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_NO_BANK_LEADER_MICROBLOCK_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_NO_BANK_LEADER_MICROBLOCK_DESC "Time in nanos spent in each state (Pack scheduled a non-empty microblock but all banks were busy?)"

#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_NO_BANK_LEADER_MICROBLOCK_OFF  (273UL)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_NO_BANK_LEADER_MICROBLOCK_NAME "pack_metric_timing_txn_no_bank_leader_microblock"
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_NO_BANK_LEADER_MICROBLOCK_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_NO_BANK_LEADER_MICROBLOCK_DESC "Time in nanos spent in each state (Pack scheduled a non-empty microblock but all banks were busy?)"

#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_BANK_LEADER_MICROBLOCK_OFF  (274UL)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_BANK_LEADER_MICROBLOCK_NAME "pack_metric_timing_no_txn_bank_leader_microblock"
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_BANK_LEADER_MICROBLOCK_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_NO_TXN_BANK_LEADER_MICROBLOCK_DESC "Time in nanos spent in each state (Pack scheduled a non-empty microblock and now has no transactions)"

#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_BANK_LEADER_MICROBLOCK_OFF  (275UL)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_BANK_LEADER_MICROBLOCK_NAME "pack_metric_timing_txn_bank_leader_microblock"
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_BANK_LEADER_MICROBLOCK_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_METRIC_TIMING_TXN_BANK_LEADER_MICROBLOCK_DESC "Time in nanos spent in each state (Pack scheduled a non-empty microblock)"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_DROPPED_FROM_EXTRA_OFF  (276UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_DROPPED_FROM_EXTRA_NAME "pack_transaction_dropped_from_extra"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_DROPPED_FROM_EXTRA_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_DROPPED_FROM_EXTRA_DESC "Transactions dropped from the extra transaction storage because it was full"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_TO_EXTRA_OFF  (277UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_TO_EXTRA_NAME "pack_transaction_inserted_to_extra"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_TO_EXTRA_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_TO_EXTRA_DESC "Transactions inserted into the extra transaction storage because pack's primary storage was full"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_FROM_EXTRA_OFF  (278UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_FROM_EXTRA_NAME "pack_transaction_inserted_from_extra"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_FROM_EXTRA_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_INSERTED_FROM_EXTRA_DESC "Transactions pulled from the extra transaction storage and inserted into pack's primary storage"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_EXPIRED_OFF  (279UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_EXPIRED_NAME "pack_transaction_expired"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_EXPIRED_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_EXPIRED_DESC "Transactions deleted from pack because their TTL expired"

#define FD_METRICS_GAUGE_PACK_AVAILABLE_TRANSACTIONS_OFF  (280UL)
#define FD_METRICS_GAUGE_PACK_AVAILABLE_TRANSACTIONS_NAME "pack_available_transactions"
#define FD_METRICS_GAUGE_PACK_AVAILABLE_TRANSACTIONS_TYPE (FD_METRICS_TYPE_GAUGE)
#define FD_METRICS_GAUGE_PACK_AVAILABLE_TRANSACTIONS_DESC "The total number of pending transactions in pack's pool that are available to be scheduled"

#define FD_METRICS_GAUGE_PACK_AVAILABLE_VOTE_TRANSACTIONS_OFF  (281UL)
#define FD_METRICS_GAUGE_PACK_AVAILABLE_VOTE_TRANSACTIONS_NAME "pack_available_vote_transactions"
#define FD_METRICS_GAUGE_PACK_AVAILABLE_VOTE_TRANSACTIONS_TYPE (FD_METRICS_TYPE_GAUGE)
#define FD_METRICS_GAUGE_PACK_AVAILABLE_VOTE_TRANSACTIONS_DESC "The number of pending simple vote transactions in pack's pool that are available to be scheduled"

#define FD_METRICS_GAUGE_PACK_PENDING_TRANSACTIONS_HEAP_SIZE_OFF  (282UL)
#define FD_METRICS_GAUGE_PACK_PENDING_TRANSACTIONS_HEAP_SIZE_NAME "pack_pending_transactions_heap_size"
#define FD_METRICS_GAUGE_PACK_PENDING_TRANSACTIONS_HEAP_SIZE_TYPE (FD_METRICS_TYPE_GAUGE)
#define FD_METRICS_GAUGE_PACK_PENDING_TRANSACTIONS_HEAP_SIZE_DESC "The maximum number of pending transactions that pack can consider.  This value is fixed at Firedancer startup but is a useful reference for AvailableTransactions and AvailableVoteTransactions."

#define FD_METRICS_COUNTER_PACK_MICROBLOCK_PER_BLOCK_LIMIT_OFF  (283UL)
#define FD_METRICS_COUNTER_PACK_MICROBLOCK_PER_BLOCK_LIMIT_NAME "pack_microblock_per_block_limit"
#define FD_METRICS_COUNTER_PACK_MICROBLOCK_PER_BLOCK_LIMIT_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_MICROBLOCK_PER_BLOCK_LIMIT_DESC "The number of times pack did not pack a microblock because the limit on microblocks/block had been reached"

#define FD_METRICS_COUNTER_PACK_DATA_PER_BLOCK_LIMIT_OFF  (284UL)
#define FD_METRICS_COUNTER_PACK_DATA_PER_BLOCK_LIMIT_NAME "pack_data_per_block_limit"
#define FD_METRICS_COUNTER_PACK_DATA_PER_BLOCK_LIMIT_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_DATA_PER_BLOCK_LIMIT_DESC "The number of times pack did not pack a microblock because it reached reached the data per block limit at the start of trying to schedule a microblock"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_OFF  (285UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_CNT  (6UL)

#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_TAKEN_OFF  (285UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_TAKEN_NAME "pack_transaction_schedule_taken"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_TAKEN_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_TAKEN_DESC "Result of trying to consider a transaction for scheduling (Pack included the transaction in the microblock)"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_CU_LIMIT_OFF  (286UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_CU_LIMIT_NAME "pack_transaction_schedule_cu_limit"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_CU_LIMIT_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_CU_LIMIT_DESC "Result of trying to consider a transaction for scheduling (Pack skipped the transaction because it would have exceeded the block CU limit)"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_FAST_PATH_OFF  (287UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_FAST_PATH_NAME "pack_transaction_schedule_fast_path"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_FAST_PATH_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_FAST_PATH_DESC "Result of trying to consider a transaction for scheduling (Pack skipped the transaction because of account conflicts using the fast bitvector check)"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_BYTE_LIMIT_OFF  (288UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_BYTE_LIMIT_NAME "pack_transaction_schedule_byte_limit"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_BYTE_LIMIT_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_BYTE_LIMIT_DESC "Result of trying to consider a transaction for scheduling (Pack skipped the transaction because it would have exceeded the block data size limit)"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_WRITE_COST_OFF  (289UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_WRITE_COST_NAME "pack_transaction_schedule_write_cost"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_WRITE_COST_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_WRITE_COST_DESC "Result of trying to consider a transaction for scheduling (Pack skipped the transaction because it would have caused a writable account to exceed the per-account block write cost limit)"

#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_SLOW_PATH_OFF  (290UL)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_SLOW_PATH_NAME "pack_transaction_schedule_slow_path"
#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_SLOW_PATH_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_TRANSACTION_SCHEDULE_SLOW_PATH_DESC "Result of trying to consider a transaction for scheduling (Pack skipped the transaction because of account conflicts using the full slow check)"

#define FD_METRICS_GAUGE_PACK_CUS_CONSUMED_IN_BLOCK_OFF  (291UL)
#define FD_METRICS_GAUGE_PACK_CUS_CONSUMED_IN_BLOCK_NAME "pack_cus_consumed_in_block"
#define FD_METRICS_GAUGE_PACK_CUS_CONSUMED_IN_BLOCK_TYPE (FD_METRICS_TYPE_GAUGE)
#define FD_METRICS_GAUGE_PACK_CUS_CONSUMED_IN_BLOCK_DESC "The number of cost units consumed in the current block, or 0 if pack is not currently packing a block"

#define FD_METRICS_HISTOGRAM_PACK_CUS_SCHEDULED_OFF  (292UL)
#define FD_METRICS_HISTOGRAM_PACK_CUS_SCHEDULED_NAME "pack_cus_scheduled"
#define FD_METRICS_HISTOGRAM_PACK_CUS_SCHEDULED_TYPE (FD_METRICS_TYPE_HISTOGRAM)
#define FD_METRICS_HISTOGRAM_PACK_CUS_SCHEDULED_DESC "The number of cost units scheduled for each block pack produced.  This can be higher than the block limit because of returned CUs."
#define FD_METRICS_HISTOGRAM_PACK_CUS_SCHEDULED_MIN  (1000000UL)
#define FD_METRICS_HISTOGRAM_PACK_CUS_SCHEDULED_MAX  (192000000UL)
#define FD_METRICS_HISTOGRAM_PACK_CUS_SCHEDULED_CVT  (FD_METRICS_CONVERTER_NONE)

#define FD_METRICS_HISTOGRAM_PACK_CUS_REBATED_OFF  (309UL)
#define FD_METRICS_HISTOGRAM_PACK_CUS_REBATED_NAME "pack_cus_rebated"
#define FD_METRICS_HISTOGRAM_PACK_CUS_REBATED_TYPE (FD_METRICS_TYPE_HISTOGRAM)
#define FD_METRICS_HISTOGRAM_PACK_CUS_REBATED_DESC "The number of compute units rebated for each block pack produced.  Compute units are rebated when a transaction fails prior to execution or requests more compute units than it uses."
#define FD_METRICS_HISTOGRAM_PACK_CUS_REBATED_MIN  (1000000UL)
#define FD_METRICS_HISTOGRAM_PACK_CUS_REBATED_MAX  (192000000UL)
#define FD_METRICS_HISTOGRAM_PACK_CUS_REBATED_CVT  (FD_METRICS_CONVERTER_NONE)

#define FD_METRICS_HISTOGRAM_PACK_CUS_NET_OFF  (326UL)
#define FD_METRICS_HISTOGRAM_PACK_CUS_NET_NAME "pack_cus_net"
#define FD_METRICS_HISTOGRAM_PACK_CUS_NET_TYPE (FD_METRICS_TYPE_HISTOGRAM)
#define FD_METRICS_HISTOGRAM_PACK_CUS_NET_DESC "The net number of cost units (scheduled - rebated) in each block pack produced."
#define FD_METRICS_HISTOGRAM_PACK_CUS_NET_MIN  (1000000UL)
#define FD_METRICS_HISTOGRAM_PACK_CUS_NET_MAX  (48000000UL)
#define FD_METRICS_HISTOGRAM_PACK_CUS_NET_CVT  (FD_METRICS_CONVERTER_NONE)

#define FD_METRICS_COUNTER_PACK_COST_MODEL_UNDERCOUNT_OFF  (343UL)
#define FD_METRICS_COUNTER_PACK_COST_MODEL_UNDERCOUNT_NAME "pack_cost_model_undercount"
#define FD_METRICS_COUNTER_PACK_COST_MODEL_UNDERCOUNT_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_COST_MODEL_UNDERCOUNT_DESC "Count of transactions that used more CUs than the cost model should have permitted them to"

#define FD_METRICS_COUNTER_PACK_DELETE_MISSED_OFF  (344UL)
#define FD_METRICS_COUNTER_PACK_DELETE_MISSED_NAME "pack_delete_missed"
#define FD_METRICS_COUNTER_PACK_DELETE_MISSED_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_DELETE_MISSED_DESC "Count of attempts to delete a transaction that wasn't found"

#define FD_METRICS_COUNTER_PACK_DELETE_HIT_OFF  (345UL)
#define FD_METRICS_COUNTER_PACK_DELETE_HIT_NAME "pack_delete_hit"
#define FD_METRICS_COUNTER_PACK_DELETE_HIT_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_PACK_DELETE_HIT_DESC "Count of attempts to delete a transaction that was found and deleted"


#define FD_METRICS_PACK_TOTAL (58UL)
extern const fd_metrics_meta_t FD_METRICS_PACK[FD_METRICS_PACK_TOTAL];
