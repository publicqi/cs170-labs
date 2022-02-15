#include "Request.h"
#include "EStore.h"
#include <stdio.h>
#include <string.h>

/*
 * ------------------------------------------------------------------
 * add_item_handler --
 *
 *      Handle an AddItemReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
add_item_handler(void *args)
{
    AddItemReq* req = (AddItemReq*)args;
    printf("Handling AddItemReq: item_id - %d, quantity - %d, price - $%.2f, discount - %.2f\n", req->item_id, req->quantity, req->price, req->discount);
    EStore* e = req->store;
    e->addItem(req->item_id, req->quantity, req->price, req->discount);
    delete req;
}

/*
 * ------------------------------------------------------------------
 * remove_item_handler --
 *
 *      Handle a RemoveItemReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
remove_item_handler(void *args)
{
    RemoveItemReq* req = (RemoveItemReq*)args;
    printf("Handling RemoveItemReq: item_id - %d\n", req->item_id);
    EStore* e = req->store;
    e->removeItem(req->item_id);
    delete req;
}

/*
 * ------------------------------------------------------------------
 * add_stock_handler --
 *
 *      Handle an AddStockReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
add_stock_handler(void *args)
{
    AddStockReq *req = (AddStockReq *) args;
    printf("Handling AddStockReq: item_id - %d, additional_stock - %d\n", req->item_id, req->additional_stock);
    EStore* e = req->store;
    e->addStock(req->item_id, req->additional_stock);
    delete req;
}

/*
 * ------------------------------------------------------------------
 * change_item_price_handler --
 *
 *      Handle a ChangeItemPriceReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
change_item_price_handler(void *args)
{
    ChangeItemPriceReq* req = (ChangeItemPriceReq *) args;
    printf("Handling ChangeItemPriceReq: item_id - %d, new_price - $%.2f\n", req->item_id, req->new_price);

    EStore* e = req->store;
    e->priceItem(req->item_id, req->new_price);
    delete req;
}

/*
 * ------------------------------------------------------------------
 * change_item_discount_handler --
 *
 *      Handle a ChangeItemDiscountReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
change_item_discount_handler(void *args)
{
    ChangeItemDiscountReq* req = (ChangeItemDiscountReq *) args;
    printf("Handling ChangeItemDiscountReq: item_id - %d, new_discount - $%.2f\n", req->item_id, req->new_discount);

    EStore* e = req->store;
    e->discountItem(req->item_id, req->new_discount);
    delete req;
}

/*
 * ------------------------------------------------------------------
 * set_shipping_cost_handler --
 *
 *      Handle a SetShippingCostReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
set_shipping_cost_handler(void *args)
{
    SetShippingCostReq* rq = (SetShippingCostReq* ) args;
    printf("Handling SetShippingCostReq: new_cost %.2f\n", rq->new_cost);
    EStore* es = rq->store;
    es->setShippingCost(rq->new_cost);
    delete rq;
}

/*
 * ------------------------------------------------------------------
 * set_store_discount_handler --
 *
 *      Handle a SetStoreDiscountReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void
set_store_discount_handler(void *args)
{
    SetStoreDiscountReq* req = (SetStoreDiscountReq *) args;
    printf("Handling SetStoreDiscount: new_discount %.2f\n", req->new_discount);

    EStore* e = req->store;
    e->setStoreDiscount(req->new_discount);
    delete req;
}

/*
 * ------------------------------------------------------------------
 * buy_item_handler --
 *
 *      Handle a BuyItemReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void
buy_item_handler(void *args)
{
    BuyItemReq* req = (BuyItemReq *) args;
    printf("Handling BuyItemReq: item_id - %d, budget - $%.2f\n", req->item_id, req->budget);

    EStore* e = req->store;
    e->buyItem(req->item_id, req->budget);
    delete req;
}

/*
 * ------------------------------------------------------------------
 * buy_many_items_handler --
 *
 *      Handle a BuyManyItemsReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void
buy_many_items_handler(void *args)
{
    BuyManyItemsReq* req = (BuyManyItemsReq *) args;

    // To resolve stdout mutex, we need to printf at once
    char buffer[0x100];
    const char* format = "Handling BuyManyItemsReq: item_ids - ";
    char* start;

    sprintf(buffer, "%s", format);

    for (size_t i = 0; i < req->item_ids.size(); i++){
        start = buffer + strlen(buffer);
        snprintf(start, (0x100 - (start - buffer)), "%d ", req->item_ids[i]);
    }
    *(char*)(buffer + strlen(buffer) - 1) = '\n';

    EStore * e = req->store;
    e->buyManyItems(&req->item_ids, req->budget);
    delete req;
}

/*
 * ------------------------------------------------------------------
 * stop_handler --
 *
 *      The thread should exit.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
stop_handler(void* args)
{
  printf("Handling StopHandlerReq: Quitting.\n");
  sthread_exit();
}

