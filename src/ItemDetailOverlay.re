let smallThreshold = 500;

module Styles = {
  open Css;
  let smallThresholdMediaQuery = styles =>
    media("(max-width: " ++ string_of_int(smallThreshold) ++ "px)", styles);
  let backdrop =
    style([
      position(absolute),
      top(zero),
      bottom(zero),
      left(zero),
      right(zero),
      backgroundColor(hex("ffffff80")),
    ]);
  let root =
    style([
      backgroundColor(hex("ffffff")),
      borderRadius(px(8)),
      position(relative),
      maxWidth(px(512)),
      boxSizing(borderBox),
      width(pct(90.)),
      boxShadow(Shadow.box(~blur=px(32), rgba(0, 0, 0, 0.2))),
      overflow(auto),
      maxHeight(vh(100.)),
      minHeight(px(224)),
      media("(max-width: 400px)", [width(pct(100.)), height(vh(95.))]),
    ]);
  let body =
    style([
      display(flexBox),
      justifyContent(center),
      padding2(~v=px(32), ~h=px(16)),
      media(
        "(max-width: 540px)",
        [paddingTop(px(24)), paddingBottom(px(24))],
      ),
      smallThresholdMediaQuery([justifyContent(flexStart)]),
    ]);
  let bodyContent =
    style([
      display(flexBox),
      flexDirection(column),
      smallThresholdMediaQuery([width(pct(100.))]),
    ]);
  let itemImageUnit =
    style([
      width(px(128)),
      marginRight(px(32)),
      smallThresholdMediaQuery([
        display(flexBox),
        marginTop(px(16)),
        marginRight(zero),
        width(auto),
      ]),
    ]);
  let itemImage =
    style([display(block), width(px(128)), height(px(128))]);
  let itemImageLabel =
    style([
      fontSize(px(16)),
      textAlign(center),
      marginTop(px(8)),
      smallThresholdMediaQuery([marginLeft(px(16)), textAlign(`left)]),
    ]);
  let itemName = style([fontSize(px(24)), marginBottom(px(8))]);
  let itemCategory = style([marginBottom(px(4))]);
  let itemCategoryLink =
    style([
      color(Colors.gray),
      textDecoration(none),
      hover([textDecoration(underline)]),
    ]);
  let itemTags = style([marginBottom(px(4))]);
  [@bs.module "./assets/tag.png"] external tagIcon: string = "default";
  let itemTagIcon =
    style([
      display(inlineBlock),
      backgroundImage(url(tagIcon)),
      width(px(16)),
      height(px(16)),
      backgroundSize(cover),
      marginRight(px(4)),
      position(relative),
      top(px(2)),
    ]);
  let itemTag =
    style([
      color(Colors.gray),
      display(inlineBlock),
      textDecoration(none),
      hover([textDecoration(underline)]),
      marginRight(px(6)),
    ]);
  let itemPriceIcon = style([marginRight(px(4))]);
  let itemPrices = style([display(flexBox)]);
  let itemPrice = style([marginRight(px(16)), whiteSpace(nowrap)]);
  let itemPriceLabel = style([color(Colors.gray), marginRight(px(4))]);
  let itemPriceValue = style([fontWeight(`num(700))]);
  let variantSection =
    style([
      marginTop(px(24)),
      marginRight(px(-16)),
      overflowX(auto),
      overflowY(hidden),
    ]);
  let variantTable =
    style([display(flexBox), flexWrap(wrap), maxWidth(px(288))]);
  let variantRow =
    style([
      display(flexBox),
      alignItems(center),
      marginRight(px(8)),
      marginBottom(px(4)),
    ]);
  let variantRowLabel = style([alignItems(flexStart)]);
  let variantLabel = style([width(px(92)), flexShrink(0.)]);
  let variantCell = style([fontSize(zero)]);
  let variantImage =
    style([
      borderRadius(px(4)),
      border(px(1), solid, transparent),
      display(inlineBlock),
      width(px(32)),
      height(px(32)),
      flexShrink(0.),
      cursor(pointer),
    ]);
  let variantImageCell =
    style([
      marginRight(px(4)),
      hover([borderColor(Colors.veryLightGray)]),
    ]);
  let variantImageWrap = style([marginRight(px(8))]);
  let variantImageSelected =
    style([important(borderColor(Colors.lightGray))]);
  let singleVariantUnit =
    style([
      borderRadius(px(4)),
      border(px(1), solid, transparent),
      cursor(pointer),
      hover([
        selector(
          "& ." ++ variantImage,
          [borderColor(Colors.veryLightGray)],
        ),
      ]),
    ]);
  let singleVariantUnitSelected =
    style([
      selector(
        "& ." ++ variantImage,
        [important(borderColor(Colors.lightGray))],
      ),
    ]);
  let patternCell = style([marginRight(px(4))]);
  let patternLabel =
    style([
      width(px(34)),
      transforms([translateX(px(22)), rotate(deg(45.))]),
      transformOrigin(zero, zero),
      whiteSpace(nowrap),
    ]);
};

let getItemDetailUrl = (~itemId, ~variant) => {
  let url = ReasonReactRouter.dangerouslyGetInitialUrl();
  "/"
  ++ Js.Array.joinWith("/", Belt.List.toArray(url.path))
  ++ (
    switch (url.search) {
    | "" => ""
    | search => "?" ++ search
    }
  )
  ++ "#i"
  ++ string_of_int(itemId)
  ++ (
    switch (variant) {
    | Some(variant) => ":" ++ string_of_int(variant)
    | None => ""
    }
  );
};

module VariantWithLabel = {
  [@react.component]
  let make = (~item: Item.t, ~variant, ~selected) => {
    <div
      onClick={_ => {
        Webapi.Dom.(
          location->Location.setHash(
            "i" ++ string_of_int(item.id) ++ ":" ++ string_of_int(variant),
          )
        )
      }}
      className={Cn.make([
        Styles.variantRow,
        Styles.singleVariantUnit,
        Cn.ifTrue(Styles.singleVariantUnitSelected, selected),
      ])}>
      <img
        src={Item.getImageUrl(~item, ~variant)}
        className={Cn.make([Styles.variantImage, Styles.variantImageWrap])}
      />
      <div className=Styles.variantLabel>
        {switch (Item.getVariantName(~item, ~variant, ())) {
         | Some(bodyName) => React.string(bodyName)
         | None => React.null
         }}
      </div>
    </div>;
  };
};

module OneDimensionVariants = {
  [@react.component]
  let make = (~item, ~a, ~variant) => {
    <div className={Cn.make([Styles.variantSection, Styles.variantTable])}>
      {Belt.Array.make(a, None)
       ->Belt.Array.mapWithIndex((i, _) => {
           <VariantWithLabel
             item
             variant
             selected={i == variant}
             key={string_of_int(i)}
           />
         })
       ->React.array}
    </div>;
  };
};

module TwoDimensionVariants = {
  [@react.component]
  let make = (~item, ~a, ~b, ~variant) => {
    <div
      className={Cn.make([
        Styles.variantSection,
        Cn.ifTrue(Styles.variantTable, b == 1),
      ])}>
      {Belt.Array.make(a, None)
       ->Belt.Array.mapWithIndex((i, _) =>
           if (b > 1) {
             <div className=Styles.variantRow key={string_of_int(i)}>
               <div className=Styles.variantLabel>
                 {switch (
                    Item.getVariantName(
                      ~item,
                      ~variant={
                        i * b;
                      },
                      ~hidePattern=true,
                      (),
                    )
                  ) {
                  | Some(bodyName) => React.string(bodyName)
                  | None => React.null
                  }}
               </div>
               {Belt.Array.make(b, None)
                ->Belt.Array.mapWithIndex((j, _) => {
                    let v = i * b + j;
                    <img
                      src={Item.getImageUrl(~item, ~variant=v)}
                      onClick={_ => {
                        Webapi.Dom.(
                          location->Location.setHash(
                            "i"
                            ++ string_of_int(item.id)
                            ++ ":"
                            ++ string_of_int(v),
                          )
                        )
                      }}
                      className={Cn.make([
                        Styles.variantImage,
                        Styles.variantImageCell,
                        Cn.ifTrue(Styles.variantImageSelected, variant == v),
                      ])}
                      key={string_of_int(j)}
                    />;
                  })
                ->React.array}
             </div>;
           } else {
             <VariantWithLabel
               item
               variant=i
               selected={i == variant}
               key={string_of_int(i)}
             />;
           }
         )
       ->React.array}
      {b > 1
         ? <div
             className={Cn.make([Styles.variantRow, Styles.variantRowLabel])}>
             <div className=Styles.variantLabel />
             {Belt.Array.make(b, None)
              ->Belt.Array.mapWithIndex((j, _) => {
                  <div className=Styles.patternCell key={string_of_int(j)}>
                    {switch (
                       Item.getVariantName(
                         ~item,
                         ~variant=j,
                         ~hideBody=true,
                         (),
                       )
                     ) {
                     | Some(patternName) =>
                       <div
                         className=Styles.patternLabel
                         style={ReactDOMRe.Style.make(
                           ~height=
                             string_of_int(Js.String.length(patternName) * 6)
                             ++ "px",
                           (),
                         )}>
                         {React.string(patternName)}
                       </div>
                     | None => React.null
                     }}
                  </div>
                })
              ->React.array}
           </div>
         : React.null}
    </div>;
  };
};

[@react.component]
let make = (~item, ~variant) => {
  let onClose = () => {
    let url = ReasonReactRouter.dangerouslyGetInitialUrl();
    ReasonReactRouter.push(
      "/"
      ++ Js.Array.joinWith("/", Belt.List.toArray(url.path))
      ++ (
        switch (url.search) {
        | "" => ""
        | search => "?" ++ search
        }
      ),
    );
  };
  let variant = variant->Belt.Option.getWithDefault(0);

  let viewportWidth = Utils.useViewportWidth();
  let itemImage =
    <div className=Styles.itemImageUnit>
      <img
        src={Item.getImageUrl(~item, ~variant)}
        className=Styles.itemImage
      />
      {if (Item.getNumVariations(~item) > 1) {
         switch (Item.getVariantName(~item, ~variant, ())) {
         | Some(variantName) =>
           <div className=Styles.itemImageLabel>
             {React.string(variantName)}
           </div>
         | None => React.null
         };
       } else {
         React.null;
       }}
    </div>;

  <div className=LoginOverlay.Styles.overlay>
    <div className=LoginOverlay.Styles.backdrop onClick={_ => onClose()} />
    <div className=Styles.root>
      <div className=Styles.body>
        {viewportWidth > 500 ? itemImage : React.null}
        <div className=Styles.bodyContent>
          <div className=Styles.itemName>
            {React.string(Item.getName(item))}
          </div>
          <div className=Styles.itemCategory>
            <Link
              path={"/?c=" ++ item.category} className=Styles.itemCategoryLink>
              {React.string(Utils.capitalizeFirstLetter(item.category))}
            </Link>
          </div>
          {if (item.tags->Js.Array.length > 0) {
             <div className=Styles.itemTags>
               <span className=Styles.itemTagIcon />
               {item.tags
                ->Belt.Array.map(tag =>
                    <Link
                      path={"/?q=" ++ tag} className=Styles.itemTag key=tag>
                      {React.string(Utils.capitalizeFirstLetter(tag))}
                    </Link>
                  )
                ->React.array}
             </div>;
           } else {
             React.null;
           }}
          {if (item.buyPrice != None && item.sellPrice != None) {
             <div className=Styles.itemPrices>
               {switch (item.buyPrice) {
                | Some(buyPrice) =>
                  <div className=Styles.itemPrice>
                    <span
                      className={Cn.make([
                        Emoji.Styles.bell,
                        Styles.itemPriceIcon,
                      ])}
                    />
                    <label className=Styles.itemPriceLabel>
                      {React.string("Buy")}
                    </label>
                    <span className=Styles.itemPriceValue>
                      {React.string(string_of_int(buyPrice))}
                    </span>
                  </div>
                | None => React.null
                }}
               {switch (item.sellPrice) {
                | Some(sellPrice) =>
                  <div className=Styles.itemPrice>
                    <span
                      className={Cn.make([
                        Emoji.Styles.bell,
                        Styles.itemPriceIcon,
                      ])}
                    />
                    <label className=Styles.itemPriceLabel>
                      {React.string("Sell")}
                    </label>
                    {React.string(string_of_int(sellPrice))}
                  </div>
                | None => React.null
                }}
             </div>;
           } else {
             React.null;
           }}
          {viewportWidth <= 500 ? itemImage : React.null}
          {switch (item.variations) {
           | Single => React.null
           | OneDimension(a) => <OneDimensionVariants item a variant />
           | TwoDimensions(a, b) => <TwoDimensionVariants item a b variant />
           }}
        </div>
      </div>
      <button
        onClick={_ => onClose()}
        className=LoginOverlay.Styles.closeButton
      />
    </div>
  </div>;
};