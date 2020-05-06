module Styles = {
  open Css;
  let recipe =
    style([marginTop(px(6)), textAlign(center), fontSize(px(12))]);
  [@bs.module "../assets/dotdotdot.png"]
  external dotdotdotIcon: string = "default";
  let ellipsisButton =
    style([
      width(px(19)),
      height(px(19)),
      backgroundColor(Colors.white),
      backgroundImage(url(dotdotdotIcon)),
      backgroundSize(cover),
      borderWidth(zero),
      cursor(pointer),
      padding(zero),
      outlineStyle(none),
      opacity(0.5),
      transition(~duration=200, "all"),
      hover([important(opacity(1.))]),
    ]);
  let ellipsisButtonOpen = style([opacity(1.)]);
  let menu =
    style([
      backgroundColor(Colors.white),
      borderRadius(px(8)),
      padding2(~v=px(6), ~h=zero),
      overflow(hidden),
      Colors.darkLayerShadow,
    ]);
  let menuItem =
    style([
      backgroundColor(Colors.white),
      borderWidth(zero),
      display(block),
      width(pct(100.)),
      textAlign(`left),
      cursor(pointer),
      padding2(~v=px(4), ~h=px(12)),
      hover([backgroundColor(Colors.faintGray)]),
    ]);
  let menuItemStar =
    style([
      marginLeft(px(6)),
      fontSize(px(10)),
      position(relative),
      top(px(-1)),
    ]);
  let menuItemRemove = style([color(Colors.red)]);
};

[@react.component]
let make = (~item: Item.t, ~userItem: User.item, ~variation, ~className) => {
  let me = UserStore.useMe()->Belt.Option.getExn;
  let ellipsisButtonRef = React.useRef(Js.Nullable.null);
  let (showEllipsisMenu, setShowEllipsisMenu) = React.useState(() => false);

  <>
    <button
      className={Cn.make([
        Styles.ellipsisButton,
        Cn.ifTrue(Styles.ellipsisButtonOpen, showEllipsisMenu),
        className,
      ])}
      onClick={_ => setShowEllipsisMenu(show => !show)}
      ref={ReactDOMRe.Ref.domRef(ellipsisButtonRef)}
    />
    {showEllipsisMenu
       ? <ReactAtmosphere.PopperLayer
           reference=ellipsisButtonRef
           onOutsideClick={() => setShowEllipsisMenu(_ => false)}
           options={
             placement: Some("bottom"),
             modifiers:
               Some([|
                 {
                   "name": "offset",
                   "options": {
                     "offset": [|0, 4|],
                   },
                 },
               |]),
           }
           render={_renderArgs =>
             <div className=Styles.menu>
               <button
                 onClick={_ => {
                   UserStore.setItemPriority(
                     ~itemId=item.id,
                     ~variant=variation,
                     ~isPriority=userItem.priorityTimestamp == None,
                   );
                   setShowEllipsisMenu(_ => false);
                 }}
                 className=Styles.menuItem>
                 {if (userItem.priorityTimestamp != None) {
                    React.string("Remove star");
                  } else {
                    <>
                      {React.string("Move to front")}
                      <span className=Styles.menuItemStar>
                        {React.string({j|⭐️|j})}
                      </span>
                    </>;
                  }}
               </button>
               {switch (userItem.status) {
                | CanCraft
                | ForTrade => React.null
                | CatalogOnly
                | Wishlist =>
                  <>
                    {!Item.isRecipe(~item)
                       ? <button
                           onClick={_ => {
                             UserStore.setItemStatus(
                               ~itemId=item.id,
                               ~variation,
                               ~status=ForTrade,
                             )
                           }}
                           className=Styles.menuItem>
                           {React.string("Move to For Trade")}
                         </button>
                       : React.null}
                    {item.recipe !== None
                       ? <button
                           onClick={_ =>
                             if (Item.isRecipe(~item)) {
                               UserStore.setItemStatus(
                                 ~itemId=
                                   Item.getItemIdForRecipe(~recipe=item),
                                 ~variation=0,
                                 ~status=CanCraft,
                               );
                               UserStore.removeItem(
                                 ~itemId=item.id,
                                 ~variation,
                               );
                             } else {
                               UserStore.setItemStatus(
                                 ~itemId=item.id,
                                 ~variation,
                                 ~status=CanCraft,
                               );
                             }
                           }
                           className=Styles.menuItem>
                           {React.string("Move to Can Craft")}
                         </button>
                       : React.null}
                    {me.enableCatalogCheckbox && userItem.status != CatalogOnly
                       ? <button
                           onClick={_ => {
                             UserStore.setItemStatus(
                               ~itemId=item.id,
                               ~variation,
                               ~status=CatalogOnly,
                             )
                           }}
                           className=Styles.menuItem>
                           {React.string(
                              Item.isRecipe(~item)
                                ? "Move DIY to Catalog" : "Move to Catalog",
                            )}
                         </button>
                       : React.null}
                  </>
                }}
               <button
                 onClick={_ => {
                   UserStore.removeItem(~itemId=item.id, ~variation)
                 }}
                 className={Cn.make([Styles.menuItem, Styles.menuItemRemove])}>
                 {React.string("Remove")}
               </button>
             </div>
           }
         />
       : React.null}
  </>;
};