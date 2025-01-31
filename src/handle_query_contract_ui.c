#include "ens_plugin.h"

void set_uint265_with_prefix(const uint8_t *amount,
                             uint8_t amount_size,
                             const char *unit,
                             char *out_buffer,
                             size_t out_buffer_size) {
    char tmp_buffer[100] = {0};

    if (uint256_to_decimal(amount, amount_size, tmp_buffer, sizeof(tmp_buffer)) == false) {
        THROW(EXCEPTION_OVERFLOW);
    }

    size_t result_len = strlen(tmp_buffer) + strlen(unit) + 1;  // +1 for the space
    if (out_buffer_size < result_len) {
        THROW(EXCEPTION_OVERFLOW);
    }

    // Concatenate the amount string, space, and unit
    snprintf(out_buffer, out_buffer_size, "%s %s", tmp_buffer, unit);
}

// Set UI for any address screen.
static void set_address_ui(ethQueryContractUI_t *msg, address_t *value) {
    // Prefix the address with `0x`.
    msg->msg[0] = '0';
    msg->msg[1] = 'x';

    // We need a random chainID for legacy reasons with `getEthAddressStringFromBinary`.
    // Setting it to `0` will make it work with every chainID :)
    uint64_t chainid = 0;

    // Get the string representation of the address stored in `context->beneficiary`. Put it in
    // `msg->msg`.
    getEthAddressStringFromBinary(
        value->value,
        msg->msg + 2,  // +2 here because we've already prefixed with '0x'.
        msg->pluginSharedRW->sha3,
        chainid);
}

static uint32_t array_to_hexstr(char *dst, size_t dstLen, const uint8_t *src, uint8_t count) {
    memset(dst, 0, dstLen);
    if (dstLen < (count * 2 + 1)) {
        return 0;
    }

    const char hexchars[] = "0123456789abcdef";
    for (uint8_t i = 0; i < count; i++, src++) {
        *dst++ = hexchars[*src >> 4u];
        *dst++ = hexchars[*src & 0x0Fu];
    }
    *dst = 0;  // terminate string

    return (uint32_t) (count * 2);
}

static void set_addr_ui(ethQueryContractUI_t *msg, address_t *address, const char *title) {
    strlcpy(msg->title, title, msg->titleLength);
    set_address_ui(msg, address);
}

static void set_bytes32_ui(ethQueryContractUI_t *msg, bytes32_t *array, const char *title) {
    strlcpy(msg->title, title, msg->titleLength);
    array_to_hexstr(msg->msg, msg->msgLength, array->value, 32);
}

static void set_bytes32_as_int_ui(ethQueryContractUI_t *msg, bytes32_t *array, const char *title) {
    strlcpy(msg->title, title, msg->titleLength);
    uint256_to_decimal(array->value, sizeof(array->value), msg->msg, msg->msgLength);
}

static void set_bytes32_as_int_unit_ui(ethQueryContractUI_t *msg,
                                       bytes32_t *array,
                                       const char *title,
                                       const char *unit) {
    strlcpy(msg->title, title, msg->titleLength);
    set_uint265_with_prefix(array->value, sizeof(array->value), unit, msg->msg, msg->msgLength);
}

static void set_name_ui(ethQueryContractUI_t *msg, name_t *name, const char *title) {
    strlcpy(msg->title, title, msg->titleLength);
    snprintf(msg->msg, msg->msgLength, "%s", name->text);
}

void handle_query_contract_ui(void *parameters) {
    ethQueryContractUI_t *msg = (ethQueryContractUI_t *) parameters;
    context_t *context = (context_t *) msg->pluginContext;

    // msg->title is the upper line displayed on the device.
    // msg->msg is the lower line displayed on the device.

    // Clean the display fields.
    memset(msg->title, 0, msg->titleLength);
    memset(msg->msg, 0, msg->msgLength);

    msg->result = ETH_PLUGIN_RESULT_OK;

    switch (context->selectorIndex) {
        case COMMIT:
            switch (msg->screenIndex) {
                case 0:
                    set_bytes32_ui(msg, &context->tx.body.commit.commitment, "Commitment");
                    break;
                default:
                    PRINTF("Received an invalid screenIndex\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
            }
            break;
        case REGISTER:
            switch (msg->screenIndex) {
                case 0:
                    set_name_ui(msg, &context->tx.body.regist.name, "Name");
                    break;
                case 1:
                    set_addr_ui(msg, &context->tx.body.regist.owner, "Owner");
                    break;
                case 2:
                    set_bytes32_as_int_unit_ui(msg,
                                               &context->tx.body.regist.duration,
                                               "Duration",
                                               "s");
                    break;
                case 3:
                    set_bytes32_ui(msg, &context->tx.body.regist.secret, "Secret");
                    break;
                default:
                    PRINTF("Received an invalid screenIndex\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
            }
            break;
        case REGISTER_WITH_CONFIG:
            switch (msg->screenIndex) {
                case 0:
                    set_name_ui(msg, &context->tx.body.regist_with_config.name, "Name");
                    break;
                case 1:
                    set_addr_ui(msg, &context->tx.body.regist_with_config.owner, "Owner");
                    break;
                case 2:
                    set_bytes32_ui(msg, &context->tx.body.regist_with_config.secret, "Secret");
                    break;
                case 3:
                    set_addr_ui(msg, &context->tx.body.regist_with_config.resolver, "Resolver");
                    break;
                case 4:
                    set_addr_ui(msg, &context->tx.body.regist_with_config.addr, "Addr");
                    break;
                default:
                    PRINTF("Received an invalid screenIndex\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
            }
            break;
        case RENEW:
            switch (msg->screenIndex) {
                case 0:
                    set_name_ui(msg, &context->tx.body.renew.name, "Name");
                    break;
                case 1:
                    set_bytes32_as_int_unit_ui(msg,
                                               &context->tx.body.renew.duration,
                                               "Duration",
                                               "s");
                    break;
                default:
                    PRINTF("Received an invalid screenIndex\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
            }
            break;
        case SET_NAME:
            switch (msg->screenIndex) {
                case 0:
                    set_name_ui(msg, &context->tx.body.set_name.name, "Name");
                    break;
                default:
                    PRINTF("Received an invalid screenIndex\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
            }
            break;
        case RENEW_ALL:
            switch (msg->screenIndex) {
                case 0:
                    set_bytes32_as_int_ui(msg, &context->tx.body.renew_all.n_names, "Total Names");
                    break;
                case 1:
                    set_bytes32_as_int_unit_ui(msg,
                                               &context->tx.body.renew_all.duration,
                                               "Duration",
                                               "s");
                    break;
                default:
                    PRINTF("Received an invalid screenIndex\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
            }
            break;
        case PROVE_AND_CLAIM:
            switch (msg->screenIndex) {
                case 0:
                    set_bytes32_as_int_ui(msg,
                                          &context->tx.body.prove_claim.n_inputs,
                                          "Total Inputs");
                    break;
                default:
                    PRINTF("Received an invalid screenIndex\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
            }
            break;
        case PROVE_AND_CLAIM_RESOLVER:
            switch (msg->screenIndex) {
                case 0:
                    set_addr_ui(msg, &context->tx.body.prove_claim_resolver.resolver, "Resolver");
                    break;
                case 1:
                    set_addr_ui(msg, &context->tx.body.prove_claim_resolver.addr, "Addr");
                    break;
                default:
                    PRINTF("Received an invalid screenIndex\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
            }
            break;
        case SET_OWNER:
            switch (msg->screenIndex) {
                case 0:
                    set_bytes32_ui(msg, &context->tx.body.set_owner.node, "Node");
                    break;
                case 1:
                    set_addr_ui(msg, &context->tx.body.set_owner.owner, "Owner");
                    break;
                default:
                    PRINTF("Received an invalid screenIndex\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
            }
            break;
        case SET_RESOLVER:
            switch (msg->screenIndex) {
                case 0:
                    set_bytes32_ui(msg, &context->tx.body.set_resolver.node, "Node");
                    break;
                case 1:
                    set_addr_ui(msg, &context->tx.body.set_resolver.resolver, "Resolver");
                    break;
                default:
                    PRINTF("Received an invalid screenIndex\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
            }
            break;
        case SET_SUBNODE:
            switch (msg->screenIndex) {
                case 0:
                    set_bytes32_ui(msg, &context->tx.body.set_resolver.node, "Node");
                    break;
                case 1:
                    set_bytes32_ui(msg, &context->tx.body.set_subnode.label, "Label");
                    break;
                case 2:
                    set_addr_ui(msg, &context->tx.body.set_subnode.owner, "Owner");
                    break;
                case 3:
                    set_addr_ui(msg, &context->tx.body.set_subnode.resolver, "Resolver");
                    break;
                case 4:
                    set_bytes32_as_int_unit_ui(msg, &context->tx.body.set_subnode.ttl, "TTL", "s");
                    break;
                default:
                    PRINTF("Received an invalid screenIndex\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
            }
            break;
    }
}
