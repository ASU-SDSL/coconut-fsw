#include "file_downlink.h"

#include <stdio.h>
#include <string.h>

#include "log.h"
#include "timing.h"
QueueHandle_t file_downlink_queue = NULL;

typedef struct {
  uint16_t window_start;
  uint16_t window_end;
  char file_path[MAX_PATH_SIZE];
  FILE *file;
  uint16_t next_seq;
  unsigned long last_ack_ms;
  unsigned long window_timer_ms;
} file_downlink_ctx_t;

static int send_next_packet(file_downlink_ctx_t *ctx) {
  if (ctx->file == NULL) {
    ctx->file = fopen(ctx->file_path, "rb");
    if (ctx->file == NULL) {
      logln_error("DL open fail .. trying again %s", ctx->file_path);
      return -1;
    }
  }

  fseek(ctx->file, ctx->next_seq * MAX_DOWNLINK_PACKET_SIZE, SEEK_SET);
  uint8_t buf[MAX_DOWNLINK_PACKET_SIZE];
  size_t read_bytes = fread(buf, 1, MAX_DOWNLINK_PACKET_SIZE, ctx->file);

  if (read_bytes == 0) {
    // error catch
    return 1;
  }

  logln_info("DL send seq %u size %zu", ctx->next_seq, read_bytes);
  return 0;
}

void start_new_transfer(file_downlink_ctx_t *ctx,
                        const file_downlink_queue_command_t *cmd) {
  const file_downlink_queue_command_init_data_t *init =
      (const file_downlink_queue_command_init_data_t *)cmd->data;

  strcpy(ctx->file_path, init->file_path);
  ctx->window_start = 0;
  ctx->window_end = WINDOW_SIZE_N - 1;
  ctx->next_seq = 0;
  ctx->last_ack_ms = tick_uptime_in_ms();
  ctx->window_timer_ms = ctx->last_ack_ms;
  ctx->file = NULL;
}

void initialize_file_downlink(char *file_path) {
  file_downlink_queue_command_init_data_t data = {0};
  strcpy(data.file_path, file_path);

  file_downlink_queue_command_t cmd;
  cmd.queue_command = FILE_DOWNLINK_INIT;
  memcpy(cmd.data, &data, sizeof(data));

  xQueueSendToBack(file_downlink_queue, &cmd, portMAX_DELAY);
}

void file_downlink_ack_command(char *file_path, uint16_t sequence_number) {
  file_downlink_queue_command_ack_data_t data = {0};
  strcpy(data.file_path, file_path);
  data.sequence_number = sequence_number;

  file_downlink_queue_command_t cmd;
  cmd.queue_command = FILE_DOWNLINK_ACK;
  // memcpy(cmd.data, &data, sizeof(data));

  xQueueSendToBack(file_downlink_queue, &cmd, portMAX_DELAY);
}

void change_max_packet_size(uint8_t new_packet_size) {
  file_downlink_queue_command_change_packet_size_data_t data = {0};
  data.new_packet_size = new_packet_size;

  file_downlink_queue_command_t cmd;
  cmd.queue_command = CHANGE_DOWNLINK_PACKET_SIZE;
  memcpy(cmd.data, &data, sizeof(data));

  xQueueSendToBack(file_downlink_queue, &cmd, portMAX_DELAY);
}

TickType_t tick_uptime_in_ms() { return ticks_to_ms(xTaskGetTickCount()); }

// void file_downlink_task(void) {
void file_downlink_task(void *unused) {
  file_downlink_ctx_t ctx;
  memset(&ctx, 0, sizeof(ctx));

  file_downlink_queue = xQueueCreate(FILE_DOWNLINK_MAX_QUEUE_ITEMS,
                                     sizeof(file_downlink_queue_command_t));

  file_downlink_queue_command_t cmd;

  while (1) {
    /* Process incoming queue messages
      run infinitely until  send_next_packet returns 1 (none left)
      defined ack->sequence_number assuming its the first packet

    */
    if (xQueueReceive(file_downlink_queue, &cmd, pdMS_TO_TICKS(10)) == pdPASS) {
      if (cmd.queue_command == FILE_DOWNLINK_INIT) {
        start_new_transfer(&ctx, &cmd);
      } else if (cmd.queue_command == FILE_DOWNLINK_ACK) {
        file_downlink_queue_command_ack_data_t *ack =
            (file_downlink_queue_command_ack_data_t *)cmd.data;

        if (ack->sequence_number >= ctx.window_start &&
            ack->sequence_number <= ctx.window_end) {
          ctx.window_start = ack->sequence_number + 1;
          ctx.window_end = ctx.window_start + WINDOW_SIZE_N - 1;
          ctx.next_seq = ctx.window_start;
          // is this needed if the queue will reset it
          ctx.last_ack_ms = tick_uptime_in_ms();
          ctx.window_timer_ms = ctx.last_ack_ms;
        }
      } else if (cmd.queue_command == CHANGE_DOWNLINK_PACKET_SIZE) {
      TODO:
      }
    }

    if (ctx.file_path[0] != '\0') {
      // according to past definition the first index should be null value
      if (ctx.next_seq <= ctx.window_end) {
        int result = send_next_packet(&ctx);

        if (result == 1) {
          logln_info("DL complete %s", ctx.file_path);
          memset(&ctx, 0, sizeof(ctx));
          continue;
        }
        if (result < 0) {
          logln_error("DL error aborting");
          memset(&ctx, 0, sizeof(ctx));
          continue;
        }

        ctx.next_seq += 1;
        // increment int index to next seq
      }

      int now = tick_uptime_in_ms();
      // using RTOS clock
      if ((now - ctx.window_timer_ms) > ACK_WINDOW_TIMEOUT) {
        ctx.next_seq = ctx.window_start;
        ctx.window_timer_ms = now;
      }

      if ((now - ctx.last_ack_ms) > ACK_TRANSFER_TIMEOUT) {
        memset(&ctx, 0, sizeof(ctx));
      }
    }
  }
}
