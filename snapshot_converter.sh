# # TODO: set this up on a cron-job that occurs every N hours

FIREDANCER_DIR=/data/snapshot_converter/firedancer/
OUTPUT_DIR=/data/snapshot_converter/snapshots/
OBJDIR=${OBJDIR:-build/native/gcc}
SOLANA_ENDPOINT=http://entrypoint2.mainnet-beta.solana.com:8899
NUM_PAGES=425
INDEX_MAX=525000000
GCLOUD_ENDPOINT=gs://firedancer-scratch/snapshot-converter/

# # Pull down latest main and remake
# cd $FIREDANCER_DIR
# git pull
# make distclean; make -j

# First download in snapshot (and incremental) into local file
# TODO: only download the latest snapshot if it is not equal to the existing one
cd $OUTPUT_DIR
rm -r *
echo "Downloading snapshot and incremental snapshot"
LOG_MAIN=/tmp/$$_snapshot_converter_log
wget --trust-server-names $SOLANA_ENDPOINT/incremental-snapshot.tar.bz2 > $LOG_MAIN 2>&1 & # FIXME: TEMPORARY
BG_PID_MAIN=$!
echo -e "Downloading main snapshot at pid $BG_PID_MAIN. Log at $LOG_MAIN"
LOG_INCREMENTAL=/tmp/$$_snapshot_converter_log
wget --trust-server-names $SOLANA_ENDPOINT/incremental-snapshot.tar.bz2 > $LOG_INCREMENTAL 2>&1 &
BG_PID_INCREMENTAL=$!
echo -e "Downloading incremental snapshot at pid $BG_PID_INCREMENTAL. Log at #$LOG_INCREMENTAL"
wait

# Convert file into funk checkpoint
echo $OUTPUT_DIR
cd $OUTPUT_DIR
echo -e $(ls)
SNAPSHOT_FILE=$(find . -name "*snapshot-*")
INCREMENTAL_FILE=$(find . -name "*incremental-snapshot-*")
echo -e snapshot file at $SNAPSHOT_FILE
echo -e incremental file at $INCREMENTAL_FILE

# Run fd_ledger ingest to turn the snapshots into a funk

# ./$FIREDANCER_DIR/$OBJDIR/bin/fd_ledger --cmd ingest --snapshot $CURRENT_DIR/$SNAPSHOT_FILE  \
#                         --incremental $CURRENT_DIR/$INCREMENTAL_FILE --funk-only 1 \
#                         --page-cnt $NUM_PAGES --index-max $INDEX_MAX --checkpt $OUTPUT_DIR/checkpt &

# # Upload snapshots in the meanwhile to google cloud storage alongside the snapshot
# gcloud storage rm -r $GCLOUD_ENDPOINT/old/
# gcloud storage mv $GCLOUD_ENDPOINT/latest/ $GCLOUD_ENDPOINT/old/
# gsutil mkdir $GCLOUD_ENDPOINT/latest/
# gsutil -o GSUtil:parallel_composite_upload_threshold=1 -m cp $OUTPUT_DIR/$SNAPSHOT_FILE $GCLOUD_ENDPOINT/latest/$SNAPSHOT_FILE
# gsutil -o GSUtil:parallel_composite_upload_threshold=1 -m cp $OUTPUT_DIR/$INCREMENTAL_FILE $GCLOUD_ENDPOINT/latest/$INCREMENTAL_FILE

# wait
# gsutil -o GSUtil:parallel_composite_upload_threshold=1 -m cp $OUTPUT_DIR/checkpt $GCLOUD_ENDPOINT/latest/checkpt

# # Delete old snapshot and checkpoint files