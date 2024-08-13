// This is an auto-generated file. To add entries, edit fd_types.json
#define FD_TYPE_NAME_COUNT 215
static char const * fd_type_names[FD_TYPE_NAME_COUNT] = {
 "fd_hash",
 "fd_pubkey",
 "fd_signature",
 "fd_gossip_ip4_addr",
 "fd_gossip_ip6_addr",
 "fd_feature",
 "fd_fee_calculator",
 "fd_hash_age",
 "fd_hash_hash_age_pair",
 "fd_block_hash_vec",
 "fd_block_hash_queue",
 "fd_fee_rate_governor",
 "fd_slot_pair",
 "fd_hard_forks",
 "fd_inflation",
 "fd_rent",
 "fd_epoch_schedule",
 "fd_rent_collector",
 "fd_stake_history_entry",
 "fd_stake_history",
 "fd_solana_account",
 "fd_vote_accounts_pair",
 "fd_vote_accounts",
 "fd_stake_accounts_pair",
 "fd_stake_accounts",
 "fd_stake_weight",
 "fd_stake_weights",
 "fd_delegation",
 "fd_delegation_pair",
 "fd_stake",
 "fd_stake_pair",
 "fd_stakes",
 "fd_stakes_stake",
 "fd_bank_incremental_snapshot_persistence",
 "fd_node_vote_accounts",
 "fd_pubkey_node_vote_accounts_pair",
 "fd_pubkey_pubkey_pair",
 "fd_epoch_stakes",
 "fd_epoch_epoch_stakes_pair",
 "fd_versioned_epoch_stakes_current",
 "fd_versioned_epoch_stakes",
 "fd_versioned_epoch_stakes_pair",
 "fd_pubkey_u64_pair",
 "fd_unused_accounts",
 "fd_deserializable_versioned_bank",
 "fd_bank_hash_stats",
 "fd_bank_hash_info",
 "fd_slot_map_pair",
 "fd_snapshot_acc_vec",
 "fd_snapshot_slot_acc_vecs",
 "fd_reward_type",
 "fd_solana_accounts_db_fields",
 "fd_reward_info",
 "fd_solana_manifest",
 "fd_rust_duration",
 "fd_poh_config",
 "fd_string_pubkey_pair",
 "fd_pubkey_account_pair",
 "fd_genesis_solana",
 "fd_sol_sysvar_clock",
 "fd_sol_sysvar_last_restart_slot",
 "fd_vote_lockout",
 "fd_lockout_offset",
 "fd_vote_authorized_voter",
 "fd_vote_prior_voter",
 "fd_vote_prior_voter_0_23_5",
 "fd_vote_epoch_credits",
 "fd_vote_block_timestamp",
 "fd_vote_prior_voters",
 "fd_vote_prior_voters_0_23_5",
 "fd_landed_vote",
 "fd_vote_state_0_23_5",
 "fd_vote_authorized_voters",
 "fd_vote_state_1_14_11",
 "fd_vote_state",
 "fd_vote_state_versioned",
 "fd_vote_state_update",
 "fd_compact_vote_state_update",
 "fd_compact_vote_state_update_switch",
 "fd_compact_tower_sync",
 "fd_tower_sync",
 "fd_tower_sync_switch",
 "fd_slot_history_inner",
 "fd_slot_history_bitvec",
 "fd_slot_history",
 "fd_slot_hash",
 "fd_slot_hashes",
 "fd_block_block_hash_entry",
 "fd_recent_block_hashes",
 "fd_slot_meta",
 "fd_clock_timestamp_vote",
 "fd_clock_timestamp_votes",
 "fd_sysvar_fees",
 "fd_sysvar_epoch_rewards",
 "fd_config_keys_pair",
 "fd_stake_config",
 "fd_feature_entry",
 "fd_firedancer_bank",
 "fd_cluster_type",
 "fd_epoch_bank",
 "fd_slot_bank",
 "fd_prev_epoch_inflation_rewards",
 "fd_vote",
 "fd_vote_init",
 "fd_vote_authorize",
 "fd_vote_authorize_pubkey",
 "fd_vote_switch",
 "fd_update_vote_state_switch",
 "fd_vote_authorize_with_seed_args",
 "fd_vote_authorize_checked_with_seed_args",
 "fd_vote_instruction",
 "fd_system_program_instruction_create_account",
 "fd_system_program_instruction_create_account_with_seed",
 "fd_system_program_instruction_allocate_with_seed",
 "fd_system_program_instruction_assign_with_seed",
 "fd_system_program_instruction_transfer_with_seed",
 "fd_system_program_instruction",
 "fd_system_error",
 "fd_stake_authorized",
 "fd_stake_lockup",
 "fd_stake_instruction_initialize",
 "fd_stake_lockup_custodian_args",
 "fd_stake_authorize",
 "fd_stake_instruction_authorize",
 "fd_authorize_with_seed_args",
 "fd_authorize_checked_with_seed_args",
 "fd_lockup_checked_args",
 "fd_lockup_args",
 "fd_stake_instruction",
 "fd_stake_meta",
 "fd_stake_flags",
 "fd_stake_state_v2_initialized",
 "fd_stake_state_v2_stake",
 "fd_stake_state_v2",
 "fd_nonce_data",
 "fd_nonce_state",
 "fd_nonce_state_versions",
 "fd_compute_budget_program_instruction_request_units_deprecated",
 "fd_compute_budget_program_instruction",
 "fd_config_keys",
 "fd_bpf_loader_program_instruction_write",
 "fd_bpf_loader_program_instruction",
 "fd_bpf_loader_v4_program_instruction_write",
 "fd_bpf_loader_v4_program_instruction",
 "fd_bpf_upgradeable_loader_program_instruction_write",
 "fd_bpf_upgradeable_loader_program_instruction_deploy_with_max_data_len",
 "fd_bpf_upgradeable_loader_program_instruction_extend_program",
 "fd_bpf_upgradeable_loader_program_instruction",
 "fd_bpf_upgradeable_loader_state_buffer",
 "fd_bpf_upgradeable_loader_state_program",
 "fd_bpf_upgradeable_loader_state_program_data",
 "fd_bpf_upgradeable_loader_state",
 "fd_frozen_hash_status",
 "fd_frozen_hash_versioned",
 "fd_lookup_table_meta",
 "fd_address_lookup_table",
 "fd_address_lookup_table_state",
 "fd_gossip_bitvec_u8_inner",
 "fd_gossip_bitvec_u8",
 "fd_gossip_bitvec_u64_inner",
 "fd_gossip_bitvec_u64",
 "fd_gossip_ping",
 "fd_gossip_ip_addr",
 "fd_gossip_prune_data",
 "fd_gossip_prune_sign_data",
 "fd_gossip_socket_addr",
 "fd_gossip_contact_info_v1",
 "fd_gossip_vote",
 "fd_gossip_lowest_slot",
 "fd_gossip_slot_hashes",
 "fd_gossip_slots",
 "fd_gossip_flate2_slots",
 "fd_gossip_slots_enum",
 "fd_gossip_epoch_slots",
 "fd_gossip_version_v1",
 "fd_gossip_version_v2",
 "fd_gossip_version_v3",
 "fd_gossip_node_instance",
 "fd_gossip_duplicate_shred",
 "fd_gossip_incremental_snapshot_hashes",
 "fd_gossip_socket_entry",
 "fd_gossip_contact_info_v2",
 "fd_crds_data",
 "fd_crds_bloom",
 "fd_crds_filter",
 "fd_crds_value",
 "fd_gossip_pull_req",
 "fd_gossip_pull_resp",
 "fd_gossip_push_msg",
 "fd_gossip_prune_msg",
 "fd_gossip_msg",
 "fd_addrlut_create",
 "fd_addrlut_extend",
 "fd_addrlut_instruction",
 "fd_repair_request_header",
 "fd_repair_window_index",
 "fd_repair_highest_window_index",
 "fd_repair_orphan",
 "fd_repair_ancestor_hashes",
 "fd_repair_protocol",
 "fd_repair_response",
 "fd_instr_error_enum",
 "fd_txn_instr_error",
 "fd_txn_error_enum",
 "fd_txn_result",
 "fd_cache_status",
 "fd_status_value",
 "fd_status_pair",
 "fd_slot_delta",
 "fd_bank_slot_deltas",
 "fd_pubkey_rewardinfo_pair",
 "fd_optional_account",
 "fd_calculated_stake_points",
 "fd_point_value",
 "fd_calculated_stake_rewards",
};
