module Styles = {
  open Css;
  let metaIcons =
    style([
      position(absolute),
      top(px(6)),
      left(px(7)),
      display(flexBox),
    ]);
  let metaIcon = style([opacity(0.), transition(~duration=200, "all")]);
  let topRightIcon =
    style([
      position(absolute),
      top(px(6)),
      right(px(8)),
      fontSize(px(13)),
      boxSizing(borderBox),
      cursor(`default),
      padding2(~v=zero, ~h=px(2)),
      height(px(24)),
      display(flexBox),
      alignItems(center),
      opacity(0.8),
      transition(~duration=200, "all"),
      hover([opacity(1.)]),
    ]);
  let ellipsisButton =
    style([position(absolute), top(px(8)), right(px(8))]);
  let catalogStatusButton =
    style([
      fontSize(px(13)),
      opacity(0.),
      cursor(`default),
      padding2(~v=zero, ~h=px(1)),
      transition(~duration=200, "all"),
      media("(hover: none)", [opacity(0.5)]),
    ]);
  let nameLink =
    style([
      color(Colors.charcoal),
      textDecoration(none),
      media("(hover: hover)", [hover([textDecoration(underline)])]),
    ]);
  let card =
    style([
      backgroundColor(hex("fffffff0")),
      borderRadius(px(8)),
      display(flexBox),
      flexDirection(column),
      alignItems(center),
      marginRight(px(16)),
      marginBottom(px(16)),
      padding3(~top=px(24), ~bottom=px(8), ~h=px(8)),
      position(relative),
      boxSizing(borderBox),
      width(Calc.(pct(50.) - px(16))),
      transition(~duration=200, "all"),
      media("(min-width: 660px)", [width(px(176))]),
      media(
        "(hover: hover)",
        [
          hover([
            selector("& ." ++ metaIcon, [opacity(1.)]),
            selector("& ." ++ topRightIcon, [opacity(1.)]),
            selector("& ." ++ catalogStatusButton, [opacity(1.)]),
            selector(
              "& ." ++ ItemImage.Styles.variantButton,
              [opacity(0.5)],
            ),
          ]),
        ],
      ),
    ]);
  let cardOnCatalogPage = style([paddingBottom(px(16))]);
  let itemImage = style([marginLeft(px(-8)), marginRight(px(-8))]);
  let name =
    style([fontSize(px(16)), marginBottom(px(4)), textAlign(center)]);
  let userItemNote = style([marginTop(px(8))]);
  let userNote =
    style([
      borderTop(px(1), solid, hex("f0f0f0")),
      unsafe("alignSelf", "stretch"),
      lineHeight(px(18)),
      marginTop(px(4)),
      padding3(~top=px(8), ~bottom=zero, ~h=px(4)),
    ]);
  let removeButton =
    style([
      position(absolute),
      top(px(10)),
      right(px(10)),
      selector("." ++ card ++ ":hover &", [opacity(0.8)]),
    ]);
  let recipe =
    style([marginTop(px(6)), textAlign(center), fontSize(px(12))]);
};

module StarIcon = {
  module Styles = {
    open Css;
    let icon =
      style([
        padding2(~v=zero, ~h=px(2)),
        display(flexBox),
        alignItems(center),
        height(px(24)),
        fontSize(px(12)),
        cursor(`default),
      ]);
  };

  [@react.component]
  let make = () => {
    <div className=Styles.icon> {React.string({j|⭐️|j})} </div>;
  };
};

[@react.component]
let make =
    (
      ~itemId,
      ~variation,
      ~userItem: User.item,
      ~list: ViewingList.t,
      ~editable,
      ~showRecipe,
      ~onCatalogPage=false,
      (),
    ) => {
  let item = Item.getItem(~itemId);
  let viewerItem = UserStore.useItem(~itemId, ~variation);

  <div
    className={Cn.make([
      Styles.card,
      Cn.ifTrue(Styles.cardOnCatalogPage, onCatalogPage),
    ])}>
    <div className=ItemCard.Styles.body>
      <ItemImage
        item
        variant=variation
        forceTooltip=true
        narrow=true
        className={Cn.make([ItemCard.Styles.itemImage, Styles.itemImage])}
      />
      <div className=Styles.name>
        <Link
          path={ItemDetailOverlay.getItemDetailUrl(
            ~itemId=item.id,
            ~variant=variation != 0 ? Some(variation) : None,
          )}
          className=Styles.nameLink>
          {React.string(Item.getName(item))}
        </Link>
      </div>
      {switch (showRecipe, item.recipe) {
       | (true, Some(recipe)) =>
         <div className=Styles.recipe>
           {recipe
            ->Belt.Array.map(((itemId, quantity)) =>
                <div key=itemId>
                  {React.string(itemId ++ " x " ++ string_of_int(quantity))}
                </div>
              )
            ->React.array}
         </div>
       | _ => React.null
       }}
    </div>
    <div className=Styles.metaIcons>
      {if (userItem.priorityTimestamp !== None) {
         <StarIcon />;
       } else {
         React.null;
       }}
      {switch (onCatalogPage, list, userItem.status) {
       | (true, _, CanCraft)
       | (true, _, ForTrade)
       | (false, Catalog, CanCraft)
       | (false, Catalog, ForTrade) =>
         <ReactAtmosphere.Tooltip
           text={React.string(
             userItem.status == ForTrade ? "For Trade" : "Can Craft",
           )}
           options={Obj.magic({"modifiers": None})}>
           {(
              ({onMouseEnter, onMouseLeave, onFocus, onBlur, ref}) =>
                <div
                  onMouseEnter
                  onMouseLeave
                  onFocus
                  onBlur
                  className=Styles.catalogStatusButton
                  ref={ReactDOMRe.Ref.domRef(ref)}>
                  {React.string(
                     userItem.status == ForTrade ? {j|🤝|j} : {j|🔨|j},
                   )}
                </div>
            )}
         </ReactAtmosphere.Tooltip>
       | _ => React.null
       }}
      {switch (onCatalogPage, item.recipe) {
       | (false, Some(recipe)) =>
         <ItemCard.RecipeIcon recipe className=Styles.metaIcon />
       | _ => React.null
       }}
      {if (!onCatalogPage && item.orderable) {
         <ItemCard.OrderableIcon className=Styles.metaIcon />;
       } else {
         React.null;
       }}
    </div>
    {editable
       ? <>
           {!onCatalogPage
              ? <UserItemNote
                  itemId={item.id}
                  variation
                  userItem
                  className=Styles.userItemNote
                  key={string_of_int(variation)}
                />
              : React.null}
           <UserItemEllipsisButton
             item
             userItem
             variation
             className=Styles.ellipsisButton
           />
         </>
       : <>
           {if (userItem.note->Js.String.length > 0) {
              <div className=Styles.userNote>
                {Emoji.parseText(userItem.note)}
              </div>;
            } else {
              React.null;
            }}
           {switch (viewerItem) {
            | Some(viewerItem) =>
              switch (list, viewerItem.status) {
              | (ForTrade, Wishlist)
              | (CanCraft, Wishlist)
              | (Wishlist, ForTrade)
              | (Wishlist, CanCraft) =>
                <ReactAtmosphere.Tooltip
                  text={React.string(
                    "In your " ++ User.itemStatusToString(viewerItem.status),
                  )}>
                  {(
                     ({onMouseEnter, onMouseLeave, ref}) =>
                       <div
                         onMouseEnter
                         onMouseLeave
                         className={Cn.make([Styles.topRightIcon])}
                         ref={ReactDOMRe.Ref.domRef(ref)}>
                         {React.string(
                            User.itemStatusToEmoji(viewerItem.status),
                          )}
                       </div>
                   )}
                </ReactAtmosphere.Tooltip>
              | _ => React.null
              }
            | None => React.null
            }}
         </>}
  </div>;
};