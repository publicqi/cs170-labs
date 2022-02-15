#include <cassert>
#include <stdio.h>

#include "EStore.h"
#define LOCK()      do{  \
                        if(fineModeEnabled()){  \
                            smutex_lock(&fine_mutex[item_id]);  \
                        }else{  \
                            smutex_lock(&mutex);  \
                        }  \
                        }while(0)
#define UNLOCK()    do{  \
                        if(fineModeEnabled()){  \
                            smutex_unlock(&fine_mutex[item_id]);  \
                            return;  \
                        }else{  \
                            smutex_unlock(&mutex);  \
                            return;  \
                        }  \
                        }while(0)

using namespace std;


Item::
Item() : valid(false)
{
}

Item::
~Item()
{
}


EStore::
EStore(bool enableFineMode)
    : fineMode(enableFineMode)
{
    smutex_init(&mutex);  // Mutex for fine mode is to lock shippingcost / discount
    if(enableFineMode){
        for(int i = 0; i < INVENTORY_SIZE; i++){
            smutex_init(&fine_mutex[i]);
        }
    }
    scond_init(&cv);
}

EStore::
~EStore()
{
   smutex_destroy(&mutex);
   scond_destroy(&cv);
}

/*
 * ------------------------------------------------------------------
 * buyItem --
 *
 *      Attempt to buy the item from the store.
 *
 *      An item can be bought if:
 *          - The store carries it.
 *          - The item is in stock.
 *          - The cost of the item plus the cost of shipping is no
 *            more than the budget.
 *
 *      If the store *does not* carry this item, simply return and
 *      do nothing. Do not attempt to buy the item.
 *
 *      If the store *does* carry the item, but it is not in stock
 *      or its cost is over budget, block until both conditions are
 *      met (at which point the item should be bought) or the store
 *      removes the item from sale (at which point this method
 *      returns).
 *
 *      The overall cost of a purchase for a single item is defined
 *      as the current cost of the item times 1 - the store
 *      discount, plus the flat overall store shipping fee.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
buyItem(int item_id, double budget)
{
    assert(!fineModeEnabled());
    smutex_lock(&mutex);

    // If item is invalid
    assert(item_id >= 0 && item_id < INVENTORY_SIZE);
    Item item = inventory[item_id];
    if(!item.valid){
        smutex_unlock(&mutex);
        return;
    }

    // Wait until has enough money and item in stock
    while (budget < item.price * (1 - item.discount) * (1 - storeDiscount) + shippingCost
        || item.quantity <= 0){
        scond_wait(&cv, &mutex);
    }

    // Do the purchase
    item.quantity --;

    smutex_unlock(&mutex);
}

/*
 * ------------------------------------------------------------------
 * buyManyItem --
 *
 *      Attempt to buy all of the specified items at once. If the
 *      order cannot be bought, give up and return without buying
 *      anything. Otherwise buy the entire order at once.
 *
 *      The entire order can be bought if:
 *          - The store carries all items.
 *          - All items are in stock.
 *          - The cost of the the entire order (cost of items plus
 *            shipping for each item) is no more than the budget.
 *
 *      If multiple customers are attempting to buy at the same
 *      time and their orders are mutually exclusive (i.e., the
 *      two customers are not trying to buy any of the same items),
 *      then their orders must be processed at the same time.
 *
 *      For the purposes of this lab, it is OK for the store
 *      discount and shipping cost to change while an order is being
 *      processed.
 *
 *      The cost of a purchase of many items is the sum of the
 *      costs of purchasing each item individually. The purchase
 *      cost of an individual item is covered above in the
 *      description of buyItem.
 *
 *      Challenge: For bonus points, implement a version of this
 *      method that will wait until the order can be fulfilled
 *      instead of giving up. The implementation should be efficient
 *      in that it should not wake up threads unecessarily. For
 *      instance, if an item decreases in price, only threads that
 *      are waiting to buy an order that includes that item should be
 *      signaled (though all such threads should be signaled).
 *
 *      Challenge: For bonus points, ensure that the shipping cost
 *      and store discount does not change while processing an
 *      order.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
buyManyItems(vector<int>* item_ids, double budget)
{
    assert(fineModeEnabled());
    double total_cost = 0.0;
    for (size_t i = 0; i < item_ids->size(); i++){
        assert(item_ids->at(i) >= 0 && item_ids->at(i) < INVENTORY_SIZE);
        // printf("buyManyItems: Acquiring lock for %d\n", item_ids->at(i));
        smutex_lock(&fine_mutex[item_ids->at(i)]);
        Item item = inventory[item_ids->at(i)];

        // Give up
        if(!item.valid || !item.quantity){
            // printf("buyManyItems: Failed to buy %d\n", item_ids->at(i));
            // Unlock all item mutex that earlier locked
            for(long j = i; j >= 0; j--){
                // printf("buyManyItems: Releasing %d\n", item_ids->at(i));
                smutex_unlock(&fine_mutex[item_ids->at(j)]);
            }
            return;
        }

        // Assume storeDiscount and shippingCost does not matter
        total_cost += item.price * (1 - item.discount) * (1 - storeDiscount) + shippingCost;
        if(total_cost > budget){
            // Unlock all item mutex that earlier locked
            for(long j = i; j >= 0; j--){
                smutex_unlock(&fine_mutex[item_ids->at(j)]);
            }
            return;
        }
    }

    // Do the purchase
    // printf("Succesfully bought ");
    for(size_t i = 0; i < item_ids->size(); i++){
        inventory[item_ids->at(i)].quantity --;
        // printf("%d ", item_ids->at(i));
        smutex_unlock(&fine_mutex[item_ids->at(i)]);
    }
}

/*
 * ------------------------------------------------------------------
 * addItem --
 *
 *      Add the item to the store with the specified quantity,
 *      price, and discount. If the store already carries an item
 *      with the specified id, do nothing.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
addItem(int item_id, int quantity, double price, double discount)
{
    // printf("Acquiring lock for %d\n", item_id);
    LOCK();
    // printf("Got lock for %d\n", item_id);

    assert(item_id >= 0 && item_id < INVENTORY_SIZE);
    // Failed to add
    if(inventory[item_id].valid){
        UNLOCK();
    }

    // Set the attributes
    inventory[item_id].valid = true;
    inventory[item_id].quantity = quantity;
    inventory[item_id].price = price;
    inventory[item_id].discount = discount;

    // printf("Releasing lock for %d\n", item_id);
    UNLOCK();
}

/*
 * ------------------------------------------------------------------
 * removeItem --
 *
 *      Remove the item from the store. The store no longer carries
 *      this item. If the store is not carrying this item, do
 *      nothing.
 *
 *      Wake any waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
removeItem(int item_id)
{
    LOCK();

    inventory[item_id].valid = false;

    UNLOCK();
}

/*
 * ------------------------------------------------------------------
 * addStock --
 *
 *      Increase the stock of the specified item by count. If the
 *      store does not carry the item, do nothing. Wake any waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
addStock(int item_id, int count)
{
    LOCK();

    inventory[item_id].quantity += count;

    // Only needs to broadcast if there is a waiting cv
    if(!fineModeEnabled()){
        scond_broadcast(&cv, &mutex);
    }

    UNLOCK();
}

/*
 * ------------------------------------------------------------------
 * priceItem --
 *
 *      Change the price on the item. If the store does not carry
 *      the item, do nothing.
 *
 *      If the item price decreased, wake any waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
priceItem(int item_id, double price)
{
    LOCK();

    double old_price = inventory[item_id].price;
    inventory[item_id].price = price;

    if(!fineModeEnabled() && old_price > price){
        scond_broadcast(&cv, &mutex);
    }

    UNLOCK();
}

/*
 * ------------------------------------------------------------------
 * discountItem --
 *
 *      Change the discount on the item. If the store does not carry
 *      the item, do nothing.
 *
 *      If the item discount increased, wake any waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
discountItem(int item_id, double discount)
{
    LOCK();

    double old_discount = inventory[item_id].discount;
    inventory[item_id].discount = discount;
    if(!fineModeEnabled() && old_discount < discount){
        scond_broadcast(&cv, &mutex);
    }

    UNLOCK();
}

/*
 * ------------------------------------------------------------------
 * setShippingCost --
 *
 *      Set the per-item shipping cost. If the shipping cost
 *      decreased, wake any waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
setShippingCost(double cost)
{
    smutex_lock(&mutex);

    double old_cost = shippingCost;
    shippingCost = cost;
    if(!fineModeEnabled() && old_cost > cost){
        scond_broadcast(&cv, &mutex);
    }

    smutex_unlock(&mutex);
}

/*
 * ------------------------------------------------------------------
 * setStoreDiscount --
 *
 *      Set the store discount. If the discount increased, wake any
 *      waiters.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void EStore::
setStoreDiscount(double discount)
{
    smutex_lock(&mutex);

    double old_discount = storeDiscount;
    storeDiscount = discount;
    if(!fineModeEnabled() && old_discount < discount){
        scond_broadcast(&cv, &mutex);
    }

    smutex_unlock(&mutex);
}


