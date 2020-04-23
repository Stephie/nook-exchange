module Styles = {
  open Css;
  let root = style([display(flexBox), flexWrap(wrap)]);
  let select =
    style([
      borderColor(hex("00000020")),
      fontSize(px(16)),
      height(px(37)),
      marginRight(px(8)),
      marginBottom(px(8)),
    ]);
  let selectSort = style([marginRight(px(16))]);
  let textInput =
    style([
      backgroundColor(Colors.white),
      border(px(1), solid, hex("00000020")),
      fontSize(px(16)),
      marginRight(px(8)),
      marginBottom(px(8)),
      padding2(~v=zero, ~h=px(12)),
      borderRadius(px(4)),
      height(px(35)),
      width(px(180)),
    ]);
  let clearFilters =
    style([
      alignSelf(flexStart),
      fontSize(px(16)),
      lineHeight(px(37)),
      paddingRight(px(16)),
    ]);
  let pager =
    style([fontSize(px(16)), lineHeight(px(32)), marginBottom(px(8))]);
  let pagerArrow =
    style([
      fontSize(px(24)),
      lineHeight(px(24)),
      textDecoration(none),
      opacity(0.8),
      padding2(~v=zero, ~h=px(8)),
      transition(~duration=200, "all"),
      hover([opacity(1.)]),
    ]);
};

type sort =
  | ABC
  | SellPriceDesc
  | SellPriceAsc
  | Category;

type mask =
  | Orderable
  | HasRecipe;

type t = {
  text: string,
  mask: option(mask),
  category: option(string),
  sort,
};

let serialize = (~filters, ~defaultSort, ~pageOffset) => {
  let p = [||];
  if (filters.sort != defaultSort) {
    p
    |> Js.Array.push((
         "s",
         switch (filters.sort) {
         | ABC => "abc"
         | SellPriceDesc => "pd"
         | SellPriceAsc => "pa"
         | Category => "cat"
         },
       ))
    |> ignore;
  };
  if (filters.text != "") {
    p |> Js.Array.push(("q", filters.text)) |> ignore;
  };
  switch (filters.mask) {
  | Some(Orderable) => p |> Js.Array.push(("orderable", "")) |> ignore
  | Some(HasRecipe) => p |> Js.Array.push(("has-recipe", "")) |> ignore
  | None => ()
  };
  switch (filters.category) {
  | Some(category) => p |> Js.Array.push(("c", category)) |> ignore
  | None => ()
  };
  if (pageOffset != 0) {
    p |> Js.Array.push(("p", string_of_int(pageOffset + 1))) |> ignore;
  };
  p;
};

let fromUrlSearch = (~urlSearch, ~defaultSort) => {
  open Belt;
  open Webapi.Url.URLSearchParams;
  let searchParams = make(urlSearch);
  (
    {
      text: (searchParams |> get("q"))->Option.getWithDefault(""),
      mask:
        searchParams |> has("orderable")
          ? Some(Orderable)
          : searchParams |> has("has-recipe") ? Some(HasRecipe) : None,
      category:
        Option.flatMap(searchParams |> get("c"), category =>
          if (Item.validCategoryStrings |> Js.Array.includes(category)) {
            Some(category);
          } else {
            None;
          }
        ),
      sort:
        switch (searchParams |> get("s")) {
        | Some("abc") => ABC
        | Some("pd") => SellPriceDesc
        | Some("pa") => SellPriceAsc
        | Some("cat") => Category
        | _ => defaultSort
        },
    },
    Option.map(searchParams |> get("p"), s => int_of_string(s) - 1)
    ->Option.getWithDefault(0),
  );
};

let doesItemMatchCategory = (~item: Item.t, ~category: string) => {
  switch (category) {
  | "furniture" =>
    Item.furnitureCategories |> Js.Array.includes(item.category)
  | "clothing" => Item.clothingCategories |> Js.Array.includes(item.category)
  | "other" => Item.otherCategories |> Js.Array.includes(item.category)
  | "recipes" => item.isRecipe
  | category => item.category == category
  };
};

let doesItemMatchFilters = (~item: Item.t, ~filters: t) => {
  (
    switch (filters.text) {
    | "" => true
    | text =>
      let fragments =
        (
          Js.String.toLowerCase(text)
          |> Js.String.splitByRe([%bs.re {|/[\s-]+/|}])
        )
        ->Belt.Array.keepMap(x => x);
      fragments->Belt.Array.every(fragment =>
        Js.String.toLowerCase(item.name) |> Js.String.includes(fragment)
      );
    }
  )
  && (
    switch (filters.mask) {
    | Some(Orderable) => item.orderable
    | Some(HasRecipe) => item.recipe !== None
    | None => true
    }
  )
  && (
    switch (filters.category) {
    | Some(category) => doesItemMatchCategory(~item, ~category)
    | None => true
    }
  );
};

let getSort = (~sort) => {
  Belt.(
    switch (sort) {
    | ABC => (
        (a: Item.t, b: Item.t) =>
          int_of_float(Js.String.localeCompare(b.name, a.name))
      )
    | SellPriceDesc => (
        (a: Item.t, b: Item.t) =>
          Option.getWithDefault(b.sellPrice, 0)
          - Option.getWithDefault(a.sellPrice, 0)
      )
    | SellPriceAsc => (
        (a: Item.t, b: Item.t) =>
          Option.getWithDefault(a.sellPrice, 0)
          - Option.getWithDefault(b.sellPrice, 0)
      )
    | Category => (
        (a: Item.t, b: Item.t) => {
          let categorySort =
            (Item.categories |> Js.Array.indexOf(a.category))
            - (Item.categories |> Js.Array.indexOf(b.category));
          if (categorySort != 0) {
            categorySort;
          } else {
            Option.getWithDefault(b.sellPrice, 0)
            - Option.getWithDefault(a.sellPrice, 0);
          };
        }
      )
    }
  );
};

module Pager = {
  [@react.component]
  let make = (~numResults, ~pageOffset, ~numResultsPerPage, ~setPageOffset) =>
    if (numResults > 8) {
      <div className=Styles.pager>
        {pageOffset > 0
           ? <a
               href="#"
               onClick={e => {
                 ReactEvent.Mouse.preventDefault(e);
                 setPageOffset(pageOffset => pageOffset - 1);
               }}
               className=Styles.pagerArrow>
               {React.string("<")}
             </a>
           : React.null}
        {React.string(
           "Showing "
           ++ string_of_int(pageOffset * numResultsPerPage + 1)
           ++ " - "
           ++ string_of_int(
                Js.Math.min_int(
                  (pageOffset + 1) * numResultsPerPage,
                  numResults,
                ),
              )
           ++ " of "
           ++ string_of_int(numResults),
         )}
        {pageOffset < numResults / numResultsPerPage
           ? <a
               href="#"
               onClick={e => {
                 ReactEvent.Mouse.preventDefault(e);
                 setPageOffset(pageOffset => pageOffset + 1);
               }}
               className=Styles.pagerArrow>
               {React.string(">")}
             </a>
           : React.null}
      </div>;
    } else {
      React.null;
    };
};

external unsafeAsHtmlInputElement: 'a => Webapi.Dom.HtmlInputElement.t =
  "%identity";

let getCategoryLabel = category => {
  switch (category) {
  | "furniture" => "All Furniture"
  | "clothing" => "All Clothing"
  | category => Utils.capitalizeFirstLetter(category)
  };
};

module CategoryButtons = {
  module CategoryStyles = {
    open Css;
    let root =
      style([
        marginBottom(px(8)),
        media("(min-width: 1200px)", [marginBottom(px(4))]),
      ]);
    let button = style([marginRight(px(8)), marginBottom(px(8))]);
    let buttonNotSelected = style([opacity(0.5), hover([opacity(1.)])]);
    let select =
      style([height(px(37)), opacity(0.5), hover([opacity(1.)])]);
    let selectSelected = style([height(px(37)), opacity(1.)]);
  };

  [@react.component]
  let make =
      (~filters: t, ~onChange, ~userItemIds: option(array(string))=?, ()) => {
    let shouldRenderCategory = category => {
      switch (userItemIds) {
      | Some(userItemIds) =>
        userItemIds->Belt.Array.some(itemId =>
          doesItemMatchCategory(~item=Item.getItem(~itemId), ~category)
        )
      | None => true
      };
    };
    let renderButton = category =>
      if (shouldRenderCategory(category)) {
        let isSelected = filters.category == Some(category);
        <Button
          onClick={_ => {
            onChange({
              ...filters,
              text: "",
              category: isSelected ? None : Some(category),
            })
          }}
          className={Cn.make([
            CategoryStyles.button,
            Cn.ifTrue(CategoryStyles.buttonNotSelected, !isSelected),
          ])}
          key=category>
          {React.string(getCategoryLabel(category))}
        </Button>;
      } else {
        React.null;
      };

    let selectCategories =
      Belt.Array.concatMany([|
        [|"wallpapers", "floors", "rugs"|],
        Item.clothingCategories,
        Item.otherCategories,
      |]);

    <div className=CategoryStyles.root>
      <Button
        onClick={_ => {onChange({...filters, category: None})}}
        className={Cn.make([
          CategoryStyles.button,
          Cn.ifTrue(
            CategoryStyles.buttonNotSelected,
            filters.category != None,
          ),
        ])}>
        {React.string("Everything!")}
      </Button>
      {renderButton("furniture")}
      {renderButton("housewares")}
      {renderButton("miscellaneous")}
      {renderButton("wall-mounted")}
      {renderButton("recipes")}
      {renderButton("clothing")}
      <select
        value={
          switch (filters.category) {
          | Some(category) =>
            selectCategories |> Js.Array.includes(category) ? category : ""
          | None => ""
          }
        }
        onChange={e => {
          let value = ReactEvent.Form.target(e)##value;
          onChange({
            ...filters,
            text: "",
            category:
              switch (value) {
              | "" => None
              | category => Some(category)
              },
          });
        }}
        className={Cn.make([
          Styles.select,
          CategoryStyles.select,
          Cn.ifTrue(
            CategoryStyles.selectSelected,
            switch (filters.category) {
            | Some(category) =>
              selectCategories |> Js.Array.includes(category)
            | None => false
            },
          ),
        ])}>
        <option value=""> {React.string("-- Other Categories")} </option>
        {selectCategories
         ->Belt.Array.mapU((. category) =>
             shouldRenderCategory(category)
               ? <option value=category key=category>
                   {React.string(getCategoryLabel(category))}
                 </option>
               : React.null
           )
         ->React.array}
      </select>
    </div>;
  };
};

module UserCategorySelector = {
  module CategoryStyles = {
    open Css;
    let select = style([height(px(37))]);
  };

  [@react.component]
  let make = (~filters: t, ~onChange, ~userItemIds: array(string)) => {
    let shouldRenderCategory = category => {
      userItemIds->Belt.Array.some(itemId =>
        doesItemMatchCategory(~item=Item.getItem(~itemId), ~category)
      );
    };

    <select
      value={Belt.Option.getWithDefault(filters.category, "")}
      onChange={e => {
        let value = ReactEvent.Form.target(e)##value;
        if (value == "") {
          onChange({...filters, category: None});
        } else {
          onChange({...filters, text: "", category: Some(value)});
        };
      }}
      className={Cn.make([Styles.select, CategoryStyles.select])}>
      <option value=""> {React.string("All categories")} </option>
      {Item.validCategoryStrings
       ->Belt.Array.mapU((. category) =>
           shouldRenderCategory(category)
             ? <option value=category key=category>
                 {React.string(Utils.capitalizeFirstLetter(category))}
               </option>
             : React.null
         )
       ->React.array}
    </select>;
  };
};

[@react.component]
let make = (~filters, ~onChange, ~userItemIds: option(array(string))=?, ()) => {
  let inputTextRef = React.useRef(Js.Nullable.null);
  let updateTextTimeoutRef = React.useRef(None);
  React.useEffect1(
    () => {
      if (filters.text == "") {
        Webapi.Dom.(
          Utils.getElementForDomRef(inputTextRef)
          ->unsafeAsHtmlInputElement
          ->HtmlInputElement.setValue("")
        );
      };
      Some(
        () => {
          switch (React.Ref.current(updateTextTimeoutRef)) {
          | Some(updateTextTimeout) =>
            Js.Global.clearTimeout(updateTextTimeout)
          | None => ()
          };
          React.Ref.setCurrent(updateTextTimeoutRef, None);
        },
      );
    },
    [|filters|],
  );

  React.useEffect0(() => {
    open Webapi.Dom;
    let onKeyDown = e => {
      switch (KeyboardEvent.key(e)) {
      | "Esc"
      | "Escape" => onChange({...filters, text: ""})
      | _ => ()
      };
    };
    window |> Window.addKeyDownEventListener(onKeyDown);
    Some(() => {window |> Window.removeKeyDownEventListener(onKeyDown)});
  });

  <div className=Styles.root>
    <input
      type_="text"
      ref={ReactDOMRe.Ref.domRef(inputTextRef)}
      placeholder="Search.. Esc to clear"
      onChange={e => {
        let value = ReactEvent.Form.target(e)##value;
        switch (React.Ref.current(updateTextTimeoutRef)) {
        | Some(updateTextTimeout) =>
          Js.Global.clearTimeout(updateTextTimeout)
        | None => ()
        };
        React.Ref.setCurrent(
          updateTextTimeoutRef,
          Some(
            Js.Global.setTimeout(
              () => {
                React.Ref.setCurrent(updateTextTimeoutRef, None);
                onChange({...filters, text: value});
              },
              300,
            ),
          ),
        );
      }}
      className=Styles.textInput
    />
    {switch (userItemIds) {
     | Some(userItemIds) =>
       <UserCategorySelector userItemIds filters onChange />
     | None => React.null
     }}
    <select
      value={
        switch (filters.mask) {
        | Some(Orderable) => "orderable"
        | Some(HasRecipe) => "has-recipe"
        | None => "none"
        }
      }
      onChange={e => {
        let value = ReactEvent.Form.target(e)##value;
        onChange({
          ...filters,
          mask:
            switch (value) {
            | "orderable" => Some(Orderable)
            | "has-recipe" => Some(HasRecipe)
            | "none" => None
            | _ => None
            },
        });
      }}
      className=Styles.select>
      <option value="none"> {React.string("No Filter")} </option>
      <option value="orderable"> {React.string("Orderable")} </option>
      <option value="has-recipe"> {React.string("Has Recipe")} </option>
    </select>
    <select
      value={
        switch (filters.sort) {
        | ABC => "abc"
        | SellPriceDesc => "sell-desc"
        | SellPriceAsc => "sell-asc"
        | Category => "category"
        }
      }
      onChange={e => {
        let value = ReactEvent.Form.target(e)##value;
        onChange({
          ...filters,
          sort:
            switch (value) {
            | "abc" => ABC
            | "sell-desc" => SellPriceDesc
            | "sell-asc" => SellPriceAsc
            | "category" => Category
            | _ => Category
            },
        });
      }}
      className={Cn.make([Styles.select, Styles.selectSort])}>
      {if (userItemIds !== None) {
         <option value="category"> {React.string("Sort: Category")} </option>;
       } else {
         React.null;
       }}
      <option value="sell-desc">
        {React.string({j|Sell Price ↓|j})}
      </option>
      <option value="sell-asc"> {React.string({j|Sell Price ↑|j})} </option>
      <option value="abc"> {React.string("A - Z")} </option>
    </select>
    {if (filters.text != "" || filters.mask != None || filters.category != None) {
       <a
         href="#"
         onClick={e => {
           ReactEvent.Mouse.preventDefault(e);
           onChange({
             text: "",
             mask: None,
             category: None,
             sort: filters.sort,
           });
         }}
         className=Styles.clearFilters>
         {React.string("Clear filters")}
       </a>;
     } else {
       React.null;
     }}
  </div>;
};