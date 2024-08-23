---
outline: deep
---

# Firedancer WebSocket API
Firedancer provides an optional [HTTP
websockets](https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API)
API for consumers to subscribe to validator information. It primarily
exists in the current form to support the Firedancer GUI.

::: warning WARNING
The API is not currently stable, is not versioned, may not exist for
long, may break or start producing incorrect data at any moment, and
should not generally be used for anything without extreme caution.
:::

## Connecting
To connect to the API, create a WebSocket client from the language of
your choice, for example in JavaScript

::: code-group

```js [client.js]
client = new WebSocket("ws://localhost:80/websocket");
```

```toml [config.toml]
[tiles]
    [tiles.http]
        gui_listen_port = 80
```

:::

The port to connect to is specified in the validator configuration TOML
file.

The API is split into various topics which will be streamed to any and
all connected clients.

## Keeping Up
The server does not drop information, slow down, or stop publishing the
stream of information if the client cannot keep up. A client that is
reading too slow and cannot keep up with incoming data stream will have
its connection forcibly closed by the server.

Most data updates are streamed in real time as the changes occur except
certain updates (performance counters like packet counters) which would
change too quickly, which are instead republished on a regular frequency
described below.

Each message is published with frequency described in the documentation
below. The meaning of these frequencies are:

| Frequency&nbsp;&nbsp;&nbsp; | Meaning |
|-----------------------------|---------|
| *Once*                      | The message is published only once, immediately after a connection is established |
| *Live*                      | The message is published live, immediately after the underlying data in the validator is changed |
| *Request*                   | The message is published in response to a specific client request |
| *1s*                        | The message is republished at regular one second intervals |
| *Once* + *Live*             | The message is published immediately after a connection is established, and then republished whenever the data is changed |

Most information related to the state of the validator is sent both
`Once` when the connection is established, and then live whenever it is
updated.

All data is encoded in JSON, with a containing envelope as follows:

```json
{
    "topic": "summary",
    "key": "cluster",
    "value": "mainnet-beta",
}
```

## Queries
Some messages are published on-demand in response to a request, and are
marked with a frequency of *Request*. To issue a query, send a websocket
frame to the server with an envelope like:

```json
{
    "topic": "slot",
    "key": "query",
    "id": 42,
    "params": {
        "slot": 285291521
    }
}
```

The `topic` and `key` correspond to the request method you wish to call.
The `id` value is an unsigned integer (must fit in `u64`) that will be
echoed back in the envelope of the response object. `params` are request
specific parameters documented for each on-demand query.

If the client issues a malformed request, it will be forcibly
disconnected. If the client issues a well-formed request for data that
the validator does not have (for example, an old slot), the query will
receive a response with a value of `null`.

## Forks
The Solana network may occasionally fork, in which case there will be
more than one active chain. When showing information derived from the
chain, the API will (unless specified otherwise) show information
reflecting the current fork choice of this validator. The current fork
choice of this validator might not be the newest, or the heaviest (most
voted on, or most likely to be chosen) fork.

For example, when showing the transactions per second (TPS) rate under
`summary.estimated_tps`, it will be calculated using the transactions
and block timings observed in the current fork. Similarly, the
`completed_slot` is the last completed slot on the current fork choice.

When the validator switches fork choice, certain of this information
will be republished to make sure it reflects the new fork choice.

## Topics

### summary
A set of high level informational fields about the validator.

#### `summary.version`
| frequency | type     | example         |
|-----------|----------|-----------------|
| *Once*    | `string` | `"0.106.11814"` |

The current version of the running validator.

#### `summary.cluster`
| frequency | type     | example        |
|-----------|----------|----------------|
| *Once*    | `string` | `"mainnet-beta"` |

One of `mainnet-beta`, `devnet`, `testnet`, `pythtest`, `pythnet`, or
`unknown`. Indicates the cluster that the validator is likely to be
running on. The cluster is guessed by looking at the genesis hash of the
chain and entrypoints that the validator connects to.

#### `summary.identity_key`
| frequency | type     | example        |
|-----------|----------|----------------|
| *Once*    | `string` | `"Fe4StcZSQ228dKK2hni7aCP7ZprNhj8QKWzFe5usGFYF"` |

The public identity key assigned to the running validator, encoded in
base58. Firedancer does not support changing the identity key of the
validator while it is running and this value does not change.

#### `summary.uptime_nanos`
| frequency | type     | example           |
|-----------|----------|-------------------|
| *Once*    | `number` |  `21785299176204` |

The length of time in nanoseconds that the validator has been running.
Running time is approximately measured since application startup, and
includes time to download a snapshot and catch up to the cluster.

### `summary.net_tile_count`
| frequency  | type     | example |
|------------|----------|---------|
| *Once*     | `number` | `1`     |

The number of `net` tiles in the validator topology.

### `summary.quic_tile_count`
| frequency  | type     | example |
|------------|----------|---------|
| *Once*     | `number` | `1`     |

The number of `quic` tiles in the validator topology.

### `summary.verify_tile_count`
| frequency  | type     | example |
|------------|----------|---------|
| *Once*     | `number` | `4`     |

The number of `verify` tiles in the validator topology.

### `summary.bank_tile_count`
| frequency  | type     | example |
|------------|----------|---------|
| *Once*     | `number` | `2`     |

The number of `bank` tiles in the validator topology.

### `summary.shred_tile_count`
| frequency  | type     | example |
|------------|----------|---------|
| *Once*     | `number` | `2`     |

The number of `shred` tiles in the validator topology.

### `summary.balance`
| frequency      | type     | example    |
|----------------|----------|------------|
| *Once* + *60s* | `number` | `21125572` |

Account balance of this validators identity key in lamports. The balance
is on the highest slot of the currently active fork of the validator.

#### `summary.root_slot`
| frequency       | type     | example     |
|-----------------|----------|-------------|
| *Once* + *Live* | `number` | `275138349` |

The last slot that was rooted. Rooted slots are fully confirmed and
irreversible, and the rooted slot will never decrease as switching fork
cannot unroot the slot. The number will not always increase by one, as
skipped slots do not update the root slot. For example, if the root slot
goes from `1001` to `1003` it means slot `1002` was skipped.

#### `summary.optimistically_confirmed_slot`
| frequency       | type     | example     |
|-----------------|----------|-------------|
| *Once* + *Live* | `number` | `275138349` |

The highest slot on the current fork that was optimistically confirmed.
Optimistic confirmation means that over two-thirds of stake have voted
to confirm the slot, and it is unlikely (although still possible, if
validators switch vote) to not become rooted.

Although rare, the `optimistically_confirmed_slot` could decrease if a
validator switches to another fork that does not have this slot.

#### `summary.completed_slot`
| frequency       | type     | example     |
|-----------------|----------|-------------|
| *Once* + *Live* | `number` | `275138349` |

The highest completed slot on the current fork choice of the validator.
The completed slot may decrease if the validator is switching forks, or
could stay the same for much more than the slot production time (400
milliseconds) if leaders are offline and not producing blocks.

#### `summary.estimated_slot`
| frequency       | type     | example     |
|-----------------|----------|-------------|
| *Once* + *Live* | `number` | `275138349` |

The estimated slot is the same as the completd slot, except it still
progresses forward even if the current leaders are skipping (not
producing) their slot. For example, if the last completed slot was
`1001` and it has been 800 milliseconds since that slot, the estimated
slot is likely to be `1003`.

#### `summary.estimated_slot_duration_nanos`
| frequency       | type     | example     |
|-----------------|----------|-------------|
| *Once* + *Live* | `number` | `450267129` |

The estimated duration of each slot on the network. This is a moving
average from the prior 750 slots, or around five minutes. Live here
means the estimate is republished whenever it changes, which is when
a new slot is confirmed on the currently active fork.

#### `summary.estimated_tps`
| frequency       | type     | example     |
|-----------------|----------|-------------|
| *Once* + *Live* | `number` | `6048` |

The estimated number of transactions per second the network is running
at. This includes vote, non-vote, and failed transactions. This is a
moving average from the prior 150 slots, or around one minute. For a
more precise view of transactions per second, the client can calculate
it from the stream of new slot data.

#### `summary.estimated_nonvote_tps`
| frequency       | type     | example     |
|-----------------|----------|-------------|
| *Once* + *Live* | `number` | `2145` |

The estimated number of non-vote transactions per second the network is
running at. The sum of the estimated vote and non-vote transactions will
be equal to the estimated total tranasactions per second. This includes
failed transactions. It is a moving average from the prior 150 slots, or
around one minute. For a more precise view of non-vote transactions per
second, the client can calculate it from the stream of new slot data.

#### `summary.estimated_vote_tps`
| frequency       | type     | example     |
|-----------------|----------|-------------|
| *Once* + *Live* | `number` | `3903`      |

The estimated number of vote transactions per second the network is
running at. The sum of the estimated vote and non-vote transactions will
be equal to the estimated total tranasactions per second. This includes
failed vote transactions. It is a moving average from the prior 150
slots, or around one minute. For a more precise view of non-vote
transactions per second, the client can calculate it from the stream of
new slot data.

#### `summary.estimated_failed_tps`
| frequency       | type     | example |
|-----------------|----------|---------|
| *Once* + *Live* | `number` | `2145`  |

The estimated number of failed vote and non-vote transactions per second
the network is running at. This is a moving average from the prior 150
slots, or around one minute. For a more precise view of non-vote
transactions per second, the client can calculate it from the stream of
new slot data.

#### `summary.live_txn_waterfall`
| frequency        | type               | example |
|------------------|--------------------|---------|
| *Once* + *100ms* | `LiveTxnWaterfall` | below   |

::: details Example

```json
{
    "topic": "summary",
    "key": "live_txn_waterfall",
    "value": {
        "next_leader_slot": 285228774,
        "waterfall": {
            "in": {
                "retained": 2014,
                "quic": 66767,
                "udp": 1054,
                "gossip": 517
            },
            "out": {
                "quic_overrun": 45,
                "quic_quic_invalid": 12,
                "quic_udp_invalid": 13,
                "verify_overrun": 2059,
                "verify_parse": 14,
                "verify_failed": 4092,
                "verify_duplicate": 128,
                "dedup_duplicate": 87,
                "pack_invalid": 14,
                "pack_retained": 1985,
                "pack_overrun": 54,
                "pack_priority": 58422,
                "bank_invalid": 14,
                "block_success": 2976,
                "block_fail": 419
            }
        }
    }
}
```

:::

**`LiveTxnWaterfall`**
| Field            | Type           | Description
|------------------|----------------|------------
| next_leader_slot | `number\|null` | The next leader slot that the transactions are being accumulated for |
| waterfall        | `TxnWaterfall` | A waterfall of transactions received since the end of the previous leader slot |

A transaction waterfall describes the transactions that are received
before and during a leader slot, and what happened to them. A typical
waterfall is that we acquire transactions from QUIC or gossip in the
lead up to (before) our leader slot, drop a few of them that fail to
verify, drop a few duplicates, drop some low priority ones that won't
fit into our block, and then successfully place some transactions into
a block. Transactions can also be received and dropped during the leader
slot, but it's important to note: the waterfall shows statistics for all
transactions since the end of our last leader slot. These are
transactions that are now eligible for palcement into the next one.

The waterfall is typically useful when viewing what happened in a past
leader slot: we want to know where transactions came from, and for what
reasons they didn't make it into the block. For example, if we received
100,000 transactions leading up to the slot, but only 6000 made it in,
what happened to the other 94,000?

The live waterfall is a special case: it's for the next slot of the
validator, rather than one that is in the past. Becuase the slot hasn't
happened yet, we know certain information: how many transactions we have
received so far from users that we could pack into our next block, how
many have expired, how many failed to verify, and so on, but we probably
won't know how many made it into the block yet, as we do when looking at
the waterfall for a block that has been published.

The waterfall should generally be balanced: total transactions in and
total transactions out will be the roughly the same, but not always
strictly. Transactions in could be more or less than transactions out
due to sampling jiter. When subtracting, be sure to account for
potential underflow.

#### `summary.live_tile_timers`
| frequency        | type          | example |
|------------------|---------------|---------|
| *Once* + *10ms*  | `TileTimer[]` | below   |

::: details Example

```json
{
    "topic": "summary",
    "key": "live_tile_timers",
    "value": [
        { "tile": "net", "kind_id": 0, "idle": 44.972112412 },
        { "tile": "quic", "kind_id": 0, "idle": 90.12 },
        { "tile": "verify", "kind_id": 0, "idle": 5.42148 },
        { "tile": "verify", "kind_id": 1, "idle": 6.24870 },
        { "tile": "verify", "kind_id": 2, "idle": 5.00158 },
        { "tile": "verify", "kind_id": 3, "idle": 8.1111556 },
        { "tile": "dedup", "kind_id": 0, "idle": 76.585 },
        { "tile": "pack", "kind_id": 0, "idle": 44.225 },
        { "tile": "bank", "kind_id": 0, "idle": 12.98 },
        { "tile": "bank", "kind_id": 1, "idle": 16.2981 },
        { "tile": "poh", "kind_id": 0, "idle": 43.857 },
        { "tile": "shred", "kind_id": 0, "idle": 14.1 },
        { "tile": "store", "kind_id": 0, "idle": 3.15716 },
        { "tile": "sign", "kind_id": 0, "idle": 93.2456 },
        { "tile": "metric", "kind_id": 0, "idle": 87.9987 }
    ]
}
```

:::

**`TileTimer`**
| Field      | Type     | Description
|------------|----------|------------
| tile       | `string` | One of `net`, `quic`, `verify`, `dedup`, `pack`, `bank`, `poh`, `shred`, `store`, `sign`, or `metric` indicating what the tile kind is
| kind_id    | `number` | A number counting up from 0 indicating the index of the tile within its kind. If there are two `verify` tiles, they will have kind_id of `0` and `1`
| idle       | `number` | Percentage idleness of the tile during the window being sampled

### epoch
Information about an epoch. Epochs are never modified once they have
been determined, so the topic only publishes a continuous stream of new
epochs as they are known. When connecting, the current and next epoch
are known, unless the validator has recently booted in which case they
may not be known and no epochs will be sent until the snapshot is loaded.
Epochs become known one epoch in advance, and will only be published
once they are confirmed (the prior epoch has fully rooted).

#### `epoch.new`
| frequency       | type            | example |
|-----------------|-----------------|-------- |
| *Once* + *Live* | `EpochSchedule` | below   |

::: details Example

```json
{
    "epoch": 636,
    "start_slot": 274752000,
    "end_slot": 275183999,
    "excluded_stake_lamports": 0,
    "staked_pubkeys": [
        "Fe4StcZSQ228dKK2hni7aCP7ZprNhj8QKWzFe5usGFYF",
        "2CeCyRoYQcctDmbXWrSUfTT4aQkGVCnArAmbdmQ5QGFi",
        "6JPDr4URdEDP5MqPgmDT6jk2nToyMUzNU27qsGxrRgKz",
        "8ri9HeWZv4Dcf4BD46pVPjmefzJLpbtfdAtyxyeG4enL",
        // ... many more ...
    ],
    "staked_lamports": [
        360,
        240,
        180,
        9991,
        // ... many more ...
    ],
    "leader_slots": [
        15,
        1542,
        761,
        // ... many more ...
    ]
}
```

:::

**`EpochSchedule`**
| Field      | Type    | Description
|------------|---------|------------
| epoch      | `number` | An identity counter for each epoch, starting at zero for the first epoch and going up |
| start_slot | `number` | The first slot (inclusive) in the epoch |
| end_slot   | `number` | The last slot (inclusive) in the epoch |
| excluded_stake_lamports | `number` | This number is almost always zero. Firedancer has a limit of 40,200 for the number of staked peer validators it can keep track of. In the unlikely event that this number is exceeded, the lowest staked peers will be forgotten, and their stake will not appear in the below lists. But is is useful to know the total stake in the epoch, so this value represents the leftover/excluded ("poisoned") amount of stake that we do not know which validator it belongs to
| staked_pubkeys | `string[]` | A list of all of validator identity keys for validators which have are staked in this epoch.  There will be at most 40,200 staked keys, after which lower staked keys will not be included |
| staked_lamports | `number[]` | A list with the same length as the `staked_pubkeys` field. `stake_lamports[ i ]` is the number of lamports staked on the pubkey `staked_pubkeys[ i ]` as of this epoch
| leader_slots | `number[]` | An array, one entry per four slots, of which pubkey in the `leader_pubkeys` array is leader for those slots. On `mainnet-beta` this array will always have a length of 108,000, which is the number of slots in an epoch divded by four.  Leader slots are in groups of four because the leader schedule is generated in such a way as to guarantee each leader gets at least four consecutive slots.  For example, to find the pubkey of the leader in slot 1000 of the epoch, it is `staked_pubkeys[ leader_slots[ 1000/4 ] ]` |

On establishing a connection two epochs are sent to the client. The
current epoch that the cluster is in, and the next epoch. From then on,
new epochs are published live as they are calculated by the validator. For
epoch T, it is published as `end_slot` in epoch T-2 is rooted. The
epoch is speculatively known as soon as `end_slot` in epoch T-2 is
compelted, rather than rooted, but no speculative epoch information is
published until the epoch is finalized by rooting the slot.

### peers
Information about validator peers from the cluster. Peer data is sourced
from gossip, the accounts database, and the on-chain configuration
program. All peer information is authenticated meaning it can only be
reported from the holder of the private key, however not all peer data
is validated or checked for correctness. In particular, data from the
gossip network and the config program is self reported by the validator
and could be empty, corrupt, filled with garbage, or malicious.

Peer information is keyed by the validator identity key. Multiple vote
accounts could in theory use the same identity keypair, although it is
not likely. Not all identities reported will have gossip data, a vote
account, or validator information published to the config program, but
all identities will have at least one of these fields reported. Once an
identity is no longer in these three data sources, it will be removed.

#### `peers.update`
| frequency      | type         | example     |
|----------------|--------------|-------------|
| *Once* + *60s* | `PeerUpdate` | below       |

::: details Example

```json
{
    "update": [
        {
            "identity_pubkey": "Fe4StcZSQ228dKK2hni7aCP7ZprNhj8QKWzFe5usGFYF",
            "gossip": {
                "version": "1.18.15",
                "feature_set": 4215500110,
                "wallclock": 0,
                "shred_version": 0,
                "sockets": {
                    "gossip": "93.119.195.160:8001",
                    "tpu": "192.64.85.26:8000",
                    // ... other sockets ...
                }
            },
            "vote": [
                {
                    "vote_pubkey": "8ri9HeWZv4Dcf4BD46pVPjmefzJLpbtfdAtyxyeG4enL",
                    "activated_stake": 5812,
                    "last_vote": 281795801,
                    "root_slot": 281795770,
                    "epoch_credits": 5917,
                    "commission": 5,
                    "delinquent": false
                }
            ],
            "info": {
                "name": "ExampleStake Firedancer 🔥💃",
                "details": "A longer description of the validator, perhaps describing the team behind it or how the node is operated",
                "website": "https://github.com/firedancer-io/firedancer",
                "icon_url": "https://firedancer-io.github.io/firedancer/fire.svg"
            }
        }
    ],
    "remove": [
        { "identity_pubkey": "8ri9HeWZv4Dcf4BD46pVPjmefzJLpbtfdAtyxyeG4enL" }
    ]
}
```

:::

**`PeerUpdateGossip`**
| Field         | Type     | Description
|---------------|----------|------------
| wallclock     | `number` | Not entirely sure yet TODO |
| shred_version | `number` | A `u16` representing the shred version the validator is configured to use. The shred version is changed when the cluster restarts, and is used to make sure the validator is talking to nodes that have participated in the same cluster restart |
| version | `string\|null` | Software version being advertised by the validator. Might be `null` if the validator is not gossiping a version, or we have received the contact information but not the version yet. The version string, if not null, will always be formatted like `major`.`minor`.`patch` where `major`, `minor`, and `patch` are `u16`s |
| feature_set | `number\|null` | First four bytes of the `FeatureSet` hash interpreted as a little endian `u32`. Might be `null` if the validator is not gossiping a feature set, or we have received the contact information but not the feature set yet |
| sockets | `[key: string]: string` | A dictionary of sockets that are advertised by the validator. `key` will be one of `gossip`, `repair`, `rpc`, `rpc_pubsub`, `serve_repair`, `serve_repair_quic`, `tpu`, `tpu_forwards`, `tpu_forwards_quic`, `tpu_quic`, `tpu_vote`, `tvu`, or `tvu_forwards`. The value is an address like `<addr>:<port>`: the location to send traffic to for this validator with the given protocol. Address might be either an IPv4 or an IPv6 address |

**`PeerUpdateVoteAccount`**
| Field       | Type     | Description
|-------------|----------|------------
| vote_pubkey | `string` | The public key of vote account, encoded in base58 |
| activated_stake | `number` | The amount of stake in lamports that is activated on this vote account for the current epoch. Warming up or cooling down stake that was delegating during this epoch is not included |
| last_vote | `number\|null` | The last vote by the vote account that was landed on chain, as seen by this validator. If the vote account has not yet landed any votes on the chain this will be `null` |
| root_slot | `number\|null` | The last slot that was rooted by the vote account, based on the vote history. If the vote account has not yet rooted any slots this will be `null` |
| epoch_credits | `number` | The number of credits earned by the vote account during the current epoch |
| delinquent | `boolean` | Whether the vote account is delinquent or not. A vote account is considered delinquent if it has not had a vote land on chain for any of the last 127 (inclusive) confirmed slots, according to this validator. If there have been less than 128 confirmed slots on the chain (it is a new chain), a validator is considered delinquent only if it has not voted yet at all |

**`PeerUpdateInfo`**
| Field       | Type     | Description
|-------------|----------|------------
| name        | `string\|null` | Self reported name of the validator, could be any string or null if there is no name set |
| details     | `string\|null` | Self reported detailed description of the validator, could be any string or null if there is no details set |
| website     | `string\|null` | Self reported website of the validator, could be any string and need not be a valid URI, or could be null if there is no website set |
| icon_url    | `string\|null` | Self reported URL of the validator icon, could be any string and need not be a valid URI, or could be null if there is no icon URI set |

**`PeerUpdate`**
| Field      | Type   | Description
|------------|--------|------------
| identity | `string` | Identity public key of the validator, encoded in base58 |
| gossip | `PeerUpdateGossip\|null` | Information reported for the validator identity over the gossip network. This is authenticated and the gossip node must have been in possession of the private key to publish gossip data as this identity. Gossip information is not validated or checked for correctness and could be set to any values by the peer |
| vote | `PeerUpdateVoteAccount[]` | Information about the vote account(s) associated with this identity key, if there are any. It is extremely unusual for multiple vote accounts to report the same identity key. Vote account information like stake and commission is derived from the accounts on chain and cannot be corrupt, invalid, or incorrect |
| info | `PeerUpdateInfo\|null` | If the validator has published self reported identifying information to the chain. This is authenticated and the operator must have been in possession of the private key to publish info as this identity. Information is not validated or checked for correctness and could be set to any values by the peer |

**`PeerRemove`**
| Field    | Type   | Description
|----------|--------|------------
| identity | `string` | Identity public key of the validator, encoded in base58 |

**`PeersUpdate`**
| Field      | Type   | Description
|------------|--------|------------
| add    | `GossipPeerUpdate[]` | List of peer validators that were added since the last update, or all of the peers for the first update after connecting |
| update | `GossipPeerUpdate[]` | List of peer validators that were changed since the last update |
| remove | `GossipPeerRemove[]` | List of peer validators that were removed since the last update |

The `gossip.update` message is republished every five seconds, with a
list of gossip peers added, removed, or updated. The list of peers is
full and includes this node itself, nodes with a different
`shred_version`, nodes publishing corrupt or bad information, and so
on.

### slot
Slots are opportunities for a leader to produce a block. A slot can be
in one of five levels, and in typical operation a slot moves through
them in normal order, starting as `incomplete` and finishing as
`finalized`.

**`SlotLevel`**
| level        | description |
|--------------|-------------|
| `incomplete` | The slot does not exist, either because the chain has not yet reached the slot or because it is still in the process of being replayed by our validator |
| `completed`  | The slot has been fully received and successfully replayed by our validator |
| `optimistically_confirmed` | The slot has been finished and successfully replayed by our validator, and more than two-thirds of stake have voted to confirm the slot |
| `rooted` | Our validator has rooted the slot and considers the slot final. This occurs when 32 subsequent slots have been built on top of it |
| `finalized` | Our validator has rooted the slot, and more than two-thirds of stake has rooted the slot, the network considers it final |

Slots are `incomplete` by default as most slots exist far in the future,
and the `incomplete` level update is not typically published. A slot
will only be explicitly marked as `incomplete` if it exists on the
currently active fork, and we switch to a fork that is slower, which has
not yet reached the slot so it no longer exists.

A slot that has become `rooted` or `finalized` cannot go backwards, and
will not become `incomplete`, `completed`, or `optimistically_confirmed`
(nor will a `finalized` slot become `rooted`), but otherwise all
transitions are valid. An `optimistically_confirmed` slot, for example,
could become `incomplete` if the validator switches to a fork where the
slot has not yet been received, although it should be exceedingly rare.
Switching from `completed` to `incomplete` is more common. Levels can
also be skipped, for example going from `incomplete` straight to
`optimistically_confirmed` (if we switched from a fork that was running
behind, to the cluster majority fork including the slot).

In addition to a level, a slot can also be either skipped or included.
All levels can be in either the skipped or included state, for example
a skipped slot that is `rooted` means that the slot is skipped and
cannot be changed. A slot in the future could be skipped, because we
know it does not build on top of the fork we have currently selected,
in which case it would be both `incomplete` and `skipped`.

Slots are either `mine` (created by this validator), or not, in which
case we are replaying a block from another validator. Slots that are
`mine` contain additional information about our performance creating the
block for that slot.

Some information is only known for blocks that have been replayed
successfully (reached the `completed` state), for example the number of
transactions in the block. This number can still be known even if we are
on a fork which skips that slot. It's possible that we are on a fork
where the slot does not yet exist, a slot could be both `skipped`, and
have a status of `incomplete`, and yet we still know a valid number of
`transactions` in the slot. Once we know information like `transactions`
it does not typically change, although can in extremely rare cases where
a leader publishes two different blocks for their leader slot, and we
initially replay one but the cluster votes on the other one.

**`SlotPublish`**
| Field      | Type      | Description |
|------------|-----------|-------------|
| slot       | `number`  | Identity of the slot, counting up from zero for the first slot in the chain |
| mine       | `boolean` | True if this validator was the leader for this slot. This will never change for a slot once it has been published, and will be aligned with the epoch information |
| skipped    | `boolean` | True if the slot was skipped. The skipped state is the state in the currently active fork of the validator. The skipped state can change if the validator switches active fork |
| level      | `string`  | One of `incomplete`, `completed`, `optimistically_confirmed`, `rooted`, or `finalized` as described above. The state is the state in the currently active fork of this validator. The state can change normally (for example, a completed slot becoming optimisitically confirmed or rooted), or also because the validator switched forks |
| transactions | `number\|null` | Total number of transactions (vote and non-vote) in the block. If the slot is not skipped, this will be non-null, but in some cases it will also be non-null even if the slot was skipped. That's because we replayed the block but selected a fork without it, but we still know how many transactions were in it |
| vote_transactions | `number\|null` | Total number of vote transactions in the block. Will always be less than or equal to `transactions`. The number of non-vote transactions is given by `transactions - vote_transactions`
| failed_transactions | `number\|null` | Total number of failed transactions (vote and non-vote) in the block. Failed transactions are those which are included in the block and were charged fees, but failed to execute successfully. This is different from dropped transations which do not pay fees and are not included in the block |
| compute_units | `number\|null`       | Total number of compute units used by the slot |

#### `slot.update`
| frequency   | type          | example |
|-------------|---------------|---------|
| *Live*      | `SlotPublish` | below   |

:::details Example

:::

#### `slot.query`
| frequency   | type           | example |
|-------------|----------------|---------|
| *Request*   | `SlotResponse` | below   |

| param | type     | description |
|-------|----------|-------------|
| slot  | `number` | The slot to query for information about |

::: details Example

```json
{
    "topic": "slot",
    "key": "query",
    "seq": 32,
    "params": {
        "slot": 285291521
    }
}
```

```json
{
    "topic": "summary",
    "key": "query",
    "value": {
        // TODO
    }
}
```

:::

**`SlotResponse`**
| Field       | Type                   | Description |
|-------------|------------------------|-------------|
| publish     | `SlotPublish`          | General information about the slot |
| waterfall   | `TxnWaterfall\|null`   | If the slot is not `mine`, will be `null`. Otherwise, a waterfall showing reasons transactions were acquired since the end of the prior leader slot |
| tile_timers | `TsTileTimers[]\|null` | If the slot is not `mine`, will be `null`. Otherwise, an array of `TsTileTimers` samples from the slot, sorted earliest to latest |

**`TxnWaterfall`**
| Field | Type              | Description |
|-------|-------------------|-------------|
| in    | `TxnWaterfallIn`  | Transactions received into the waterfall |
| out   | `TxnWaterfallOut` | Transactions sent out of the waterfall |

**`TxnWaterfallIn`**
| Field    | Type     | Description |
|----------|----------|-------------|
| retained | `number` | Transactions were received during or prior to an earlier leader slot, but weren't executed and were retained inside the validator to potentially be included in a later slot |
| quic     | `number` | A QUIC transaction was received. The stream does not have to successfully complete |
| udp      | `number` | A non-QUIC UDP transaction was received |
| gossip   | `number` | A gossipped vote transaction was received from a gossip peer |

**`TxnWaterfallOut`**
| Field             | Type     | Description |
|-------------------|----------|-------------|
| quic_overrun      | `number` | Transactions were dropped because the QUIC tile couldn't keep with incoming network packets. It is unclear how many transactions would have been produced by the fragments from net that were overrun, and this counter (along with the corresponding counter for the `in` side) assumes one tranaction per dropped packet |
| quic_quic_invalid | `number` | Transactions were dropped because the QUIC tile decided that incoming QUIC packets were not valid. It is unclear how many transactions would have been produced by the packets that were invalid, and this counter (along with the corresponding counter for the `in` side) assumes one tranaction per invalid packet |
| quic_udp_invalid  | `number` | Transactions were dropped because the QUIC tile decided that incoming non-QUIC (regular UDP) packets were not valid. |
| verify_overrun    | `number` | Transactions were dropped because the verify tiles could not verify them quickly enough |
| verify_parse      | `number` | Transactions were dropped because they were malformed and failed to parse |
| verify_failed     | `number` | Transactions were dropped because signature verification failed |
| verify_duplicate  | `number` | Transactions were dropped because the verify tiles determined that they had already been processed |
| dedup_duplicate   | `number` | Transactions were dropped because the dedup tile determined that they had already been processed |
| pack_invalid      | `number` | Transactions were dropped because pack determined they would never execute. Reasons can include the transaction expired, requested too many compute units, or was too large to fit in a block |
| pack_retained     | `number` | Transactions were retained inside the validator memory because they were not high enough priority to make it into a prior block we produced, but have not yet expired. We might include the transactions in a future block |
| pack_leader_slow  | `number` | Transactions were dropped while leader because the bank tiles could not execute them quickly enough, pack will drop the lowest priority transactions first |
| pack_wait_full    | `number` | Transactions were dropped while we were waiting for our leader slot because we ran out of memory to store them. All incoming transactions are dropped without regard for the priority |
| bank_invalid      | `number` | Transactions were dropped because a bank tile could not execute them enough to charge fees. Failed transactions can still pay fees and be included in a block, but invalid transactions do not make it to a block. Reasons can include insufficient fee payer balance, or invalid address lookup tables |
| block_success     | `number` | Transactions made it into a block, and execution succeeded |
| block_failure     | `number` | Transactions made it into a block, but execution failed |

**`TsTileTimers`**
| Field             | Type          | Description |
|-------------------|---------------|-------------|
| timestamp_nanos   | `number`      | A timestamp of when the tile timers were sampled, nanoseconds since the UNIX epoch |
| tile_timers       | `TileTimer[]` | A list of all tile timing information at the given sample timestamp |