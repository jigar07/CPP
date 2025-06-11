Coupon

# Flows:
    * Seller should be able to create a coupon
    * Buyer should be able to apply a coupon

# Requirements
* Coupon will have a code through which the user will apply it.
* It will a validity
* It can give % discount or flat amount discount.
* It can be either seller specific, product specific, global.
* You should be able to create a coupon which is applicable only if you buy more than x products.
  * Minimum cart value.

# Thought Process
- Classes
- Properties
- Top down methods:
  - User gives string as coupon input. To convert to Coupon class CouponController added. Then call CouponService from controller
  - CouponService
    - createCoupon - create and store coupon in repository
    - applyCoupon - method added
      - Check coupon is valid or not
      - `ICouponMatchRule`:
        - isApplicable: check whether coupon is applicable to Product, seller or global
        - apply: if isApplicable returns true then apply method is called
          - In this method check whether coupon can be applied to cart or not.
          - for CouponApplicableTo.GLOBAL, returns true as it is global coupon which can be applied to any item
          - for CouponApplicableTo.PRODUCT, check whether coupon's -> offer's-> applicable product has items of the cart or not
          - for CouponApplicableTo.SELLER, check whether cart's seller matches to coupon's -> offer's-> seller
      - `ICouponRewardRule`:
        - isApplicable: check coupon's -> offer's -> offertype matches ICouponRewardRule offertype
        - apply: If above return true then return amount after applying offer
    - applyCouponPerProduct - method added

# Notes
- Try to solve company provided question (Can find it on enginebogie, or leetcode).
- Udit usually do Unit test or manual test a lot
- Coupon system should be extensible else multiple requirements could come. But don't try to include everything even in your company. Discuss with PM or interviewer
- Don't implement things which are not scoped down. (If small things then fine, but it is large then it become complex)
- Don't worry about where things should be(write somewhere and move forward). You might get confused and might get stuck. Our goal should be move forward
- For Udit main concern is OCP. He usually use OCP most of case
- Call out to interviewer that this I am keeping as out of scope
- Big classes are fine if it is not creating any issue
- Whenever you create Price(amount, currency), Quantity(quantity, unit) then better to create POJO
- To apply coupon, there are two option. 1. Take as input. 2. Find out best coupon.
  - For interview you will choose simpler approach. Hence, we will go with providing coupon as input

# Design
## Classes
- Coupon(Coupon is core class), Cart, Offer, Product, Seller, User
## Properties of classes
- Coupon
  - TimeRange validity(POJO), code, OfferType (later removed), isDeleted, User createdBy, CouponApplicableTo, Offer, `List<User> applicableSellers` added
    - Offer or OfferType: one of this will be used
  - isActive method inside Coupon. isLive method inside TimeRange
  - For enginebogie TimeRange is used in coupon as well calendar
  - isActive is derived from other properties. So, it is not added as parameter instead method is added
  - applicableCount is not in requirement
- Offer
  - CouponApplicableTo, List<User> (looks like later replaced by List<Seller>, List<Product>, OfferType, offerPercent, offerFlat added
  - `CouponApplicableTo`, `offerPercent, offerFlat` are breaking OCP
  - So Offer is not following OCP
- Coupon and Offer are core classes. For remaining models just single integer added for ID
## Methods (top-down method)
- CouponService
  - createCoupon method
    - validation on coupon values (During interview do not worry about this. Just call out). `isInPast` added in coupon
    - `ICouponRepository` added to store coupon. (List<Coupon> can be stored in CouponService also but better to store in Respository to make it OCP compliant. In case tomorrow we want to store in DB or REDIS then it will become easier to implement)
  - ApplyCoupon (This is interesting part)
    - `Cart`, `Product` class added
      - Product has many properties. But we are writing properties which will be used for coupon service only.
      - `getTotalPrice` method added in Cart
    - ApplyCoupon method receives `Cart` and `Coupon`.
      - User gives string as coupon input. To convert to Coupon class CouponController added. (Note: in interview it is ok to send as string directly to the service instead of adding controller class)
      - Now need to do: check the applicability, validity, and then applicableToChecking
        - For applicability need to check product and seller
      - "ApplyCoupon logic 1" added (For all the product coupon we should be able to apply)
        - Now, this is not OCP compliant because if new type of applicability comes then we need to change `Offer` class and `CouponService` class. If new discount(offerType) comes then also it is not OCP compliant
        - Added `applyCouponPerProduct` method to apply coupon on single product. Note: this method is not OCP compliant
      - To solve this `ICouponMatchRule` added with method `isApplicable` and `apply`. While writing this got into issue like not able to get clear idea or get confused then stop thinking about that and start writing implementation of class
        - Started writing `ProductCouponMatchRule`
        - We created single interface for both applicability and reward (Apply coupon - offer). But both are two different things and independent. (When things are related at that time you do reverse ISP. Here, applicability and offer are independent). MatchRule does not care about reward
        - So, `ICouponRewardRule` added
        - Strategy does two things: 1. strategy applicable or not. 2. Do work required in strategy (apply method)
        - Instead of having reward based logic in `apply`, we will check whether all the items in carts have same productType or not
      - `List<ICouponMatchRule> couponMatchRules` and `List<ICouponRewardRule> couponRewardRules` added in CouponService which will solve OCP problem
      - For validity we can have `Or`, `And` condition. Similar problem we solved in ecommerce filter, ride sharding

### ApplyCoupon logic 1
``` java
final offer Offer = coupon.getOffer()
if(offer.getApplicableTo() == couponApplicableTo.SELLER) {
  for (Product cartProduct: cart.getProductList()) {
    if(!offer().getApplicableSellers().contains(cartProduct.getSeller()) {
      throw new RunTimeException("Invalid Coupon")
    }
  }
}

if(offer.getApplicableTo() == couponApplicableTo.SELLER) {
  for (Product cartProduct: cart.getProductList()) {
    if(!offer().getApplicableProducts().contains(cartProduct) &&
      throw new RunTimeException("Invalid Coupon")
    }
  }
}

// if applicable, then apply the offer
Double finalPrice = 0.0
if(offer.getOfferType() == OfferType.FLAT_DISCOUNT) {
  finalPrice = totalPrice * (100 - offer.getOfferPercent() * totalPrice)
} else {
  finalPrice = totalPrice - offer.getOfferFlat();
}

```